/* Eviorment customizations
 *  * disabledProperties
 *  * disableUnittest
 *  * disableIntegration
 *  * disableRegression
 *  * disableCopyright
 *  * disableDoxygen
 *  
 *  * offModules
 *  * onModules
 *  * opts
 */

@NonCPS
def getChangeString(build) {
    def MAX_MSG_LEN = 100
    def changeString = ""
    build.changeSets.each {entries -> 
        entries.each { entry -> 
            changeString += "${new Date(entry.timestamp).format("yyyy-MM-dd HH:mm:ss")} "
            changeString += "[${entry.commitId.take(8)}] ${entry.author}: ${entry.msg.take(MAX_MSG_LEN)}\n"
        }
    }
    return changeString ?: " - No new changes"
}

@NonCPS
def getAffectedFiles(build) {
    files = []
    build.changeSets.each {entries -> 
        entries.each { entry -> files += entry.affectedFiles }
    }
    files.unique()
    return files
}

def defaultProperties() {
    def params = [
        parameters([
            booleanParam(
                defaultValue: false, 
                description: 'Do a clean build', 
                name: 'Clean_Build'
            ),
            booleanParam(
                defaultValue: true, 
                description: 'Use ccache to speed up build', 
                name: 'Use_Ccache'
            ),
            booleanParam(
                defaultValue: false, 
                description: 'Prints all the cmake variables to the log', 
                name: 'Print_CMake_Variables'
            ),
            choice(
                // The first will be default
                choices: "Release\nDebug\nMinSizeRel\nRelWithDebInfo\n", 
                description: 'Select build configuration', 
                name: 'Build_Type'
            )
        ])
    ]
    return params
}

def log(env = [], fun) {
    withEnv(['TERM=xterm'] + env) {
        ansiColor {
            timestamps {
                fun()
            }
        }
    }
}

def cmd(stageName, dirName, env = [], fun) {
    stage(stageName) {
        dir(dirName) {
            log(env) {
                fun()
            }
        }
    }
}

def printMap(String name, def map) {
    println name + ":\n" + map?.collect{"${it.key.padLeft(30)} = ${it.value}"}?.join("\n") ?: ''
}

// this uses global pipeline var pullRequest
def setLabel(def state, String label, Boolean add) {
    try {
        if (add) {
            state.addLabel(label)
        } else {
            state.removeLabel(label)
        }
    } catch (e) {
        println "Error adding label: ${label} add: ${add}"
        println e.toString()
    }      
}

def checked(def state, String label, Boolean fail, Closure fun) {
    try {
        fun()
        setLabel(state, "J: " + label  + " Failure", false)
    } catch (e) {
        setLabel(state, "J: " + label  + " Failure", true)
        state.errors += label
        if (fail) {
            state.build.result = Result.FAILURE.toString()
            throw e
        } else {
            println e.toString()
            state.build.result = Result.UNSTABLE.toString()
        }
    }
}

def wrap(def state, String reportSlackChannel, Closure fun) {
    try {
        fun()
        state.build.result = state.errors.isEmpty() ? 'SUCCESS' : 'UNSTABLE'
    } catch (e) {
        state.build.result = 'FAILURE'
        println "State:  ${state.build.result}"
        println "Errors: ${state.errors.join(", ")}"
        println "Except: ${e}"
        throw e
    } finally {
        if (!reportSlackChannel.isEmpty()) slack(state, reportSlackChannel)
        if (!state.errors.isEmpty()) {
            println "Errors in: ${state.errors.join(", ")}"
            state.build.description = "Errors in: ${state.errors.join(' ')}"
        } 
    }
}

def filterfiles() {
    dir('build') {
        sh 'cp compile_commands.json compile_commands_org.json'
        sh 'python3 ../inviwo/tools/jenkins/filter-compilecommands.py'
    }
}

def format(def state, repos) {
    stage("Format Tests") {
        dir('build') {
            String master = state.env.Master_Build?.equals("true")? '--master' : ''
            String binary = state.env.CLANG_FORMAT ? '-binary ' + state.env.CLANG_FORMAT : ''
            sh "python3 ../inviwo/tools/jenkins/check-format.py ${master} ${binary} ${repos.join(' ')}"
            if (fileExists('clang-format-result.diff')) {
                String format_diff = readFile('clang-format-result.diff')
                setLabel(state, 'J: Format Test Failure', !format_diff.isEmpty())

                publishHTML([
                    allowMissing: false,
                    alwaysLinkToLastBuild: false,
                    keepAll: false,
                    reportDir: '.',
                    reportFiles: 'clang-format-result.diff',
                    reportName: 'Format'
                ])
            }
        }
    }
}

def warn(def state, refjob = 'daily/appleclang') {
    cmd('Warn Tests', 'inviwo') {
        checked(state, 'Warn Tests', false) {
            recordIssues qualityGates: [[threshold: 1, type: 'NEW', unstable: true]], 
                         referenceJobName: refjob, 
                         sourceCodeEncoding: 'UTF-8', 
                         tools: [clang(name: 'Clang')] 
        }
    }
}

def unittest(def state) {
    if(state.env.disableUnittest) return
    cmd('Unit Tests', 'build/bin', ['DISPLAY=:' + state.display]) {
        checked(state, "Unit Test", false) {
            sh '''
                rc=0
                for unittest in inviwo-unittests-*
                do ./${unittest} || rc=$?
                done
                exit ${rc}
            '''
        }
    }
}

def integrationtest(def state) {
    if(state.env.disableIntegration) return
    cmd('Integration Tests', 'build/bin', ['DISPLAY=:' + state.display]) {
        checked(state, 'Integration Test', false) {
            sh './inviwo-integrationtests'
        }
    }
}

def regression(def state, modulepaths) {
    if(state.env.disableRegression) return
    cmd('Regression Tests', 'regress', ['DISPLAY=:' + state.display]) {
        checked(state, 'Regression Test', false) {
            sh """
                python3 ../inviwo/tools/regression.py \
                        --config ../build/pyconfig.ini \
                        --build_type ${state.env.Build_Type?:"Release"} \
                        --header ${state.env.JENKINS_HOME}/inviwo-config/header.html \
                        --output . \
                        --summary \
                        --modules ${modulepaths.join(' ')}
            """        
        }
    }
    publishHTML([
        allowMissing: true,
        alwaysLinkToLastBuild: true,
        keepAll: false,
        reportDir: 'regress',
        reportFiles: 'report.html',
        reportName: 'Regression'
    ])
}

def copyright(def state, extraPaths = []) {
    if(state.env.disableCopyright) return
    stage('Copyright Check') {
        log() {
            checked(state, "Copyright Test", false) {
                sh "python3 inviwo/tools/refactoring/check-copyright.py ./inviwo ${extraPaths.join(' ')}"
            }
        }
    }   
}

def doxygen(def state) {
    if(state.env.disableDoxygen) return
    cmd('Doxygen', 'build', ['DISPLAY=:' + state.display]) {
        checked(state, "Doxygen", false) {
            sh 'ninja DOXY-Inviwo'
        }
        publishHTML([
            allowMissing: true,
            alwaysLinkToLastBuild: true,
            keepAll: false,
            reportDir: 'docs/inviwo/html',
            reportFiles: 'index.html',
            reportName: 'Doxygen'
        ])
    }    
}

def slack(def state, channel) {
    stage('Slack') {
        echo "result: ${state.build.currentResult}"
        def res2color = [(Result.SUCCESS) : 'good', (Result.UNSTABLE) : 'warning' , (Result.FAILURE) : 'danger' ]
        def color = res2color.containsKey(state.build.result) ? res2color[state.build.result] : 'warning'
        def errors = !state.errors.isEmpty() ? "Errors in: ${state.errors.join(" ")}\n" : ""
        slackSend(
            color: color, 
            channel: channel, 
            message: "Branch: ${state.env.BRANCH_NAME}\n" + 
                     "Status: ${state.build.result}\n" + 
                     "Job: ${state.env.BUILD_URL} \n" + 
                     "Regression: ${state.env.JOB_URL}Regression_Report/\n" + 
                     errors +
                     "Changes: " + getChangeString(state.build)
        )
    }
}

def cmake(Map args = [:]) {
    return "cmake -G Ninja " +
        (args.printCMakeVars ? " -LA " : "") +
        (args.opts?.collect{" -D${it.key}=${it.value}"}?.join('') ?: "") + 
        (args.modulePaths ? " -DIVW_EXTERNAL_MODULES=\"" + args.modulePaths.join(";") + "\"" : "" ) +
        (args.onModules?.collect{" -DIVW_MODULE_${it.toUpperCase()}=ON"}?.join('') ?: "") +
        (args.offModules?.collect{" -DIVW_MODULE_${it.toUpperCase()}=OFF"}?.join('') ?: "") +
        " ../inviwo"
}

def clean() {
    echo "Clean build, removing build folder"
    sh "rm -rf build"
}

Map defaultCMakeOptions(String buildType) {
    return [
        "CMAKE_EXPORT_COMPILE_COMMANDS" : "ON",
        "CMAKE_BUILD_TYPE" : buildType,
        "IVW_CMAKE_DEBUG" : "ON",
        "IVW_DOXYGEN_PROJECT" : "ON",
        "BUILD_SHARED_LIBS" : "ON",
        "IVW_TINY_GLFW_APPLICATION" : "ON",
        "IVW_TINY_QT_APPLICATION" : "ON",
        "IVW_UNITTESTS" : "ON",
        "IVW_UNITTESTS_RUN_ON_BUILD" : "OFF",
        "IVW_INTEGRATION_TESTS" : "ON",
        "IVW_RUNTIME_MODULE_LOADING" : "OFF"
    ]
}
Map ccacheOption() {
    return [
        "CMAKE_CXX_COMPILER_LAUNCHER" : "ccache",
    ]
}
Map envCMakeOptions(env) {
    def opts = (env.QT_PATH ? ["CMAKE_PREFIX_PATH"  : env.QT_PATH] : [:]) +
               (env.OpenCL_LIBRARY     ? [OpenCL_LIBRARY     : env.OpenCL_LIBRARY] :  [:]) +
               (env.OpenCL_INCLUDE_DIR ? [OpenCL_INCLUDE_DIR : env.OpenCL_INCLUDE_DIR] : [:])
    return opts
}


//Args state, opts, modulePaths, onModules, offModules
def build(Map args = [:]) {
    dir('build') {
        printMap("Options", args.opts)
        println "External:\n  ${args.modulePaths?.join('\n  ')?:""}"
        println "Modules On:\n  ${args.onModules?.join('\n  ')?:""}"
        println "Modules Off:\n  ${args.offModules?.join('\n  ')?:""}"
        log {
            checked(args.state, 'Build', true) {
                sh """
                    ccache --zero-stats
                    # tell ccache where the project root is
                    export CCACHE_BASEDIR=${args.state.env.WORKSPACE}           
                    ${cmake(args)}
                    ninja
                    ccache --show-stats
                """
            }
        }
    }    
}

// Args: 
// * state map of global variables (env, params, pull, errors)
// * modulePaths list of paths to module folders (optional)
// * opts Map of extra CMake options (optional)
// * onModules List of extra module to enable (optional)
// * offModules List of modules to disable (optional)
def buildStandard(Map args = [:]) {
    stage('Build') {
        if (args.state.env.Clean_Build?.equals("true")) clean()
        def defaultOpts = defaultCMakeOptions(args.state.env.Build_Type?:"Release")
        defaultOpts.putAll(envCMakeOptions(args.state.env))
        if (!args.state.env.Use_Ccache?.equals("false")) defaultOpts.putAll(ccacheOption())
        if (args.state.env.opts) {
            def envopts = args.state.env.opts.tokenize(';').collect{it.tokenize('=')}.collectEntries()
            defaultOpts.putAll(envopts)
        }
        if (args.opts) defaultOpts.putAll(args.opts)
        args.opts = defaultOpts
        args.printCMakeVars = args.state.env.Print_CMake_Variables?.equals("true")

        if (args.state.env.offModules) args.offModules += args.state.env.offModules.tokenize(';')
        if (args.state.env.onModules) args.onModules += args.state.env.onModules.tokenize(';')

        build(args)
    }
}

return this
