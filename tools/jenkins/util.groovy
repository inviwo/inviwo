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

// this uses global pipeline vars env and pullRequest
def setLabel(String label, Boolean add) {
    if (env.CHANGE_ID) {
        if (add) {
            println("Add label ${label}")
            pullRequest.addLabels([label])
        } else {
            println("Remove label ${label}")
            pullRequest.removeLabel(label)
        }
    }
}

def checked(String label, Closure fun) {
    try {
        fun()
        setLabel(label, false)
    } catch (e) {
        setLabel(label, true)
        throw e
    }
}


def filterfiles() {
    dir('build') {
        sh 'cp compile_commands.json compile_commands_org.json'
        sh 'python3 ../inviwo/tools/jenkins/filter-compilecommands.py'
    }
}

def format() {
    stage("Format Tests") {
        dir('build') {
            sh 'python3 ../inviwo/tools/jenkins/check-format.py'
            if (fileExists('clang-format-result.diff')) {
                String format_diff = readFile('clang-format-result.diff')
                setLabel('J: Format Test Failure', !format_diff.isEmpty())
            }
        }
    }
}

def warn(refjob = 'inviwo/master') {
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

def unittest(display = 0) {
    cmd('Unit Tests', 'build/bin', ['DISPLAY=:' + display]) {
        checked("J:Unit Test Failure") {
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

def integrationtest(display = 0) {
    cmd('Integration Tests', 'build/bin', ['DISPLAY=:' + display]) {
        checked('J: Integration Test Failure') {
            sh './inviwo-integrationtests'
        }
    }
}

def regression(build, env, modulepaths, display = 0) {
    cmd('Regression Tests', 'regress', ['DISPLAY=:' + display]) {
        try {
            sh """
                python3 ../inviwo/tools/regression.py \
                        --inviwo ../build/bin/inviwo \
                        --header ${env.JENKINS_HOME}/inviwo-config/header.html \
                        --output . \
                        --modules ${modulepaths.join(' ')}
            """
            setlabel('J: Regression Test Failure', false)
        } catch (e) {
            setlabel('J: Regression Test Failure', true)
            // Mark as unstable, if we mark as failed, the report will not be published.
            build.result = 'UNSTABLE'
        }
    }
}

def copyright(extraPaths = []) {
    stage('Copyright Check') {
        log() {
            sh "python3 inviwo/tools/refactoring/check-copyright.py ./inviwo ${extraPaths.join(' ')}"
        }
    }   
}

def doxygen(display = 0) {
    cmd('Doxygen', 'build', ['DISPLAY=:' + display]) {
        sh 'ninja DOXY-Inviwo'
    }    
}

def publish() {
    stage('Publish') {
        publishHTML([
            allowMissing: true,
            alwaysLinkToLastBuild: true,
            keepAll: false,
            reportDir: 'regress',
            reportFiles: 'report.html',
            reportName: 'Regression Report'
        ])
        publishHTML([
            allowMissing: true,
            alwaysLinkToLastBuild: true,
            keepAll: false,
            reportDir: 'build/doc/inviwo/html',
            reportFiles: 'index.html',
            reportName: 'Doxygen Documentation'
        ])
    }
}

def slack(build, env, channel) {
    stage('Slack') {
        echo "result: ${build.result}"
        def res2color = ['SUCCESS' : 'good', 'UNSTABLE' : 'warning' , 'FAILURE' : 'danger' ]
        def color = res2color.containsKey(build.result) ? res2color[build.result] : 'warning'
        slackSend(
            color: color, 
            channel: channel, 
            message: "Branch: ${env.BRANCH_NAME}\n" + \
                     "Status: ${build.result}\n" + \
                     "Job: ${env.BUILD_URL} \n" + \
                     "Regression: ${env.JOB_URL}Regression_Report/\n" + \
                     "Changes: " + getChangeString(build) 
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
        "IVW_RUNTIME_MODULE_LOADING" : "ON"
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


//Args opts, modulePaths, onModules, offModules
def build(Map args = [:]) {
    dir('build') {
        println("Options: ${args.opts.inject('', {res, item -> res + '\n  ' + item.key + ' = ' + item.value})}")
        println("External: ${args.modulePaths.inject('', {res, item -> res + '\n  ' + item})}")
        println("Modules On: ${args.onModules.inject('', {res, item -> res + '\n  ' + item})}")
        println("Modules Off: ${args.offModules.inject('', {res, item -> res + '\n  ' + item})}")
        log {
            checked('J:Build Failure') {
                sh """
                    ccache -z # reset ccache statistics
                    # tell ccache where the project root is
                    export CCACHE_BASEDIR=${env.WORKSPACE}/build
                            
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
// * params The global pipeline variable
// * env The global pipeline variable
// * modulePaths list of paths to module folders (optional)
// * opts Map of extra CMake options (optional)
// * onModules List of extra module to enable (optional)
// * offModules List of modules to disable (optional)
def buildStandard(Map args = [:]) {
    assert args.params, "Argument params must be supplied"
    stage('Build') {
        clean(args.params)
        def defaultOpts = defaultCMakeOptions(args.params['Build Type'])
        if (args.env) defaultOpts.putAll(envCMakeOptions(args.env))
        if (args.params['Use ccache']) defaultOpts.putAll(ccacheOption())
        if (args.opts) defaultOpts.putAll(args.opts)
        args.opts = defaultOpts
        args.printCMakeVars = args.params['Print CMake Variables']
        build(args)
    }
}

return this
