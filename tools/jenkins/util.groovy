@NonCPS
def getChangeString(build) {
    def MAX_MSG_LEN = 100
    def changeString = ""

    echo "Gathering SCM changes"
    def changeLogSets = build.rawBuild.changeSets
    for (int i = 0; i < changeLogSets.size(); i++) {
        def entries = changeLogSets[i].items
        for (int j = 0; j < entries.length; j++) {
            def entry = entries[j]
            truncated_msg = entry.msg.take(MAX_MSG_LEN)
            changeString += "${new Date(entry.timestamp).format("yyyy-MM-dd HH:mm:ss")} "
            changeString += "[${entry.commitId.take(8)}] ${entry.author}: ${truncated_msg}\n"
        }
    }

    if (!changeString) {
        changeString = " - No new changes"
    }
    return changeString
}

def defaultProperties() {
    return [
        parameters([
            booleanParam(
                defaultValue: false, 
                description: 'Do a clean build', 
                name: 'Clean Build'
            ),
            booleanParam(
                defaultValue: true, 
                description: 'Disable ccache', 
                name: 'Use ccache'
            ),
            booleanParam(
                defaultValue: false, 
                description: 'Prints all the cmake variables to the log', 
                name: 'Print CMake Variables'
            ),
            choice(
                choices: "Release\nDebug\nMinSizeRel\nRelWithDebInfo\n", // The first will be default
                description: 'Select build configuration', 
                name: 'Build Type'
            )
        ]),
        pipelineTriggers([
            [$class: 'GitHubPushTrigger']
        ])
    ]
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

// this uses global pipeline var pullRequest
def setLabel(def state, String label, Boolean add) {
    if (add) {
        try {
            state.addLabel(label)
        } catch (e) {
            println("Error adding label")
        }
    } else {
        try {
            state.removeLabel(label)
        } catch (e) {
            println("Error adding label")
        }
    }       
}

def checked(def state, String label, Closure fun) {
    try {
        fun()
        setLabel(state, "J:" + label  + " Failure", false)
    } catch (e) {
        setLabel(state, "J:" + label  + " Failure", true)
        state.errors += "Failure in ${label}"
        throw e
    }
}


def filterfiles() {
    dir('build') {
        sh 'cp compile_commands.json compile_commands_org.json'
        sh 'python3 ../inviwo/tools/jenkins/filter-compilecommands.py'
    }
}

def format(def state) {
    stage("Format Tests") {
        dir('build') {
            sh 'python3 ../inviwo/tools/jenkins/check-format.py'
            if (fileExists('clang-format-result.diff')) {
                String format_diff = readFile('clang-format-result.diff')
                setLabel(state, 'J: Format Test Failure', !format_diff.isEmpty())
            }
        }
    }
}

def warn(def state, refjob = 'inviwo/master') {
    stage("Warn Tests") {
        dir('build') {
            // disabled for now, has some macro issues.
            //sh '''cppcheck --enable=all --inconclusive --xml --xml-version=2 \
            //      --project=compile_commands.json 2> cppcheck.xml'''    
        }    
        dir('inviwo') {
            recordIssues failedNewAll: 0, referenceJobName: refjob, sourceCodeEncoding: 'UTF-8', 
                tools: [gcc4(name: 'GCC', reportEncoding: 'UTF-8'), 
                        clang(name: 'Clang', reportEncoding: 'UTF-8')]
            //recordIssues referenceJobName: refjob, sourceCodeEncoding: 'UTF-8', 
            //    tools: [cppCheck(name: 'CPPCheck', pattern: '../build/cppcheck.xml')]
        }
    }
}

def unittest(def state) {
    cmd('Unit Tests', 'build/bin', ['DISPLAY=:' + state.display]) {
        checked(state, "Unit Test") {
            sh '''
                rc=0
                for unittest in inviwo-unittests-*
                    do echo ==================================
                    echo Running: ${unittest}
                    ./${unittest} || rc=$?
                done
                exit ${rc}
            '''
        }
    }
}

def integrationtest(def state) {
    cmd('Integration Tests', 'build/bin', ['DISPLAY=:' + state.display]) {
        checked(state, 'Integration Test') {
            sh './inviwo-integrationtests'
        }
    }
}

def regression(def state, modulepaths) {
    try {
        cmd('Regression Tests', 'regress', ['DISPLAY=:' + state.display]) {
            checked(state, 'Regression Test') {
                sh """
                    python3 ../inviwo/tools/regression.py \
                            --config ../build/pyconfig.ini \
                            --build_type ${state.params['Build Type']} \
                            --header ${state.env.JENKINS_HOME}/inviwo-config/header.html \
                            --output . \
                            --slice 1:3 \
                            --modules ${modulepaths.join(' ')}
                """        
            }
        }
    } catch (e) {
        // Mark as unstable, if we mark as failed, the report will not be published.
        state.build.result = 'UNSTABLE'
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
    stage('Copyright Check') {
        log() {
            checked(state, "Copyright Test") {
                sh "python3 inviwo/tools/refactoring/check-copyright.py ./inviwo ${extraPaths.join(' ')}"
            }
        }
    }   
}

def doxygen(def state) {
    cmd('Doxygen', 'build', ['DISPLAY=:' + state.display]) {
        checked(state, "Doxygen") {
            sh 'ninja DOXY-Inviwo'
        }
        publishHTML([
            allowMissing: true,
            alwaysLinkToLastBuild: true,
            keepAll: false,
            reportDir: 'doc/inviwo/html',
            reportFiles: 'index.html',
            reportName: 'Doxygen'
        ])
    }    
}

def publish() {
}

def slack(def state, channel) {
    stage('Slack') {
        echo "result: ${state.build.result}"
        def res2color = ['SUCCESS' : 'good', 'UNSTABLE' : 'warning' , 'FAILURE' : 'danger' ]
        def color = res2color.containsKey(state.build.result) ? res2color[state.build.result] : 'warning'
        def errors = !state.errors.isEmpty() ? "Errors:\n ${state.errors.inject("", {res, item -> res + item + '\n'})}" : ""
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

def cmake(Map opts, List modulePaths, List onModules, List offModules, Boolean printVars = False) {
    return "cmake -G Ninja " +
        (printVars ? " -LA " : "") +
        opts.inject("", {res, item -> res + " -D" + item.key + "=" + item.value}) + 
        (modulePaths ? " -DIVW_EXTERNAL_MODULES=" + modulePaths.join(";") : "" ) +
        onModules.inject("", {res, item -> res + " -D" + "IVW_MODULE_" + item + "=ON"}) +
        offModules.inject("", {res, item -> res + " -D" + "IVW_MODULE_" + item + "=OFF"}) + 
        " ../inviwo"
}

def clean(params) {
    if (params['Clean Build']) {
        echo "Clean build, removing build folder"
        sh "rm -r build"
    }
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
    def res = [:]
    if (env.QT_PATH) res["CMAKE_PREFIX_PATH"] = env.QT_PATH
    if (env.OpenCL_LIBRARY) res["OpenCL_LIBRARY"] = env.OpenCL_LIBRARY
    if (env.OpenCL_INCLUDE_DIR) res["OpenCL_INCLUDE_DIR"] = env.OpenCL_INCLUDE_DIR
    return res
}


//Args state, opts, modulePaths, onModules, offModules
def build(Map args = [:]) {
    dir('build') {
        println("Options: ${args.opts.inject('', {res, item -> res + '\n  ' + item.key + ' = ' + item.value})}")
        println("External: ${args.modulePaths.inject('', {res, item -> res + '\n  ' + item})}")
        println("Modules On: ${args.onModules.inject('', {res, item -> res + '\n  ' + item})}")
        println("Modules Off: ${args.offModules.inject('', {res, item -> res + '\n  ' + item})}")
        log {
            checked(args.state, 'Build') {
                sh """
                    ccache -z # reset ccache statistics
                    # tell ccache where the project root is
                    export CCACHE_BASEDIR=${args.state.env.WORKSPACE}/build
                            
                    ${cmake(args.opts, args.modulePaths, args.onModules, args.offModules, 
                            args.printCMakeVars)}
    
                    ninja
    
                    ccache -s # print ccache statistics
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
    assert args.state.params : "Argument params must be supplied"
    stage('Build') {
        clean(args.state.params)
        def defaultOpts = defaultCMakeOptions(args.state.params['Build Type'])
        if (args.state.env) defaultOpts.putAll(envCMakeOptions(args.state.env))
        if (args.state.params['Use ccache']) defaultOpts.putAll(ccacheOption())
        if (args.opts) defaultOpts.putAll(args.opts)
        args.opts = defaultOpts
        args.printCMakeVars = args.state.params['Print CMake Variables']
        build(args)
    }
}

return this
