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

def warn(refjob = 'inviwo/master') {
    stage("Warn Tests") {
        recordIssues failedNewAll: 1, referenceJobName: refjob, sourceCodeEncoding: 'UTF-8', 
            tools: [gcc4(name: 'GCC', reportEncoding: 'UTF-8'), 
                    cppCheck(reportEncoding: 'UTF-8'), 
                    clangTidy(reportEncoding: 'UTF-8'), 
                    clang(name: 'Clang', reportEncoding: 'UTF-8')]
    }
}

def unittest(display = 0) {
    cmd('Unit Tests', 'build/bin', ['DISPLAY=:' + display]) {
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

def integrationtest(display = 0) {
    cmd('Integration Tests', 'build/bin', ['DISPLAY=:' + display]) {
        sh './inviwo-integrationtests'
    }
}

def regression(build, env, display = 0) {
    cmd('Regression Tests', 'regress', ['DISPLAY=:' + display]) {
        try {
            sh """
                python3 ../inviwo/tools/regression.py \
                        --inviwo ../build/bin/inviwo \
                        --header ${env.JENKINS_HOME}/inviwo-config/header.html \
                        --output . \
                        --repos ../inviwo
            """
        } catch (e) {
            // Mark as unstable, if we mark as failed, the report will not be published.
            build.result = 'UNSTABLE'
        }
    }
}

def copyright() {
    cmd('Copyright Check', 'inviwo') {
        sh 'python3 tools/refactoring/check-copyright.py .'
    }    
}

def doxygen(display = 0) {
    cmd('Doxygen', 'build', ['DISPLAY=:' + display]) {
        sh 'ninja DOXY-ALL'
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

def slack(build, env) {
    stage('Slack') {
        echo "result: ${build.result}"
        def res2color = ['SUCCESS' : 'good', 'UNSTABLE' : 'warning' , 'FAILURE' : 'danger' ]
        def color = res2color.containsKey(build.result) ? res2color[build.result] : 'warning'
        slackSend(
            color: color, 
            channel: "#jenkins-branch-pr", 
            message: "Inviwo branch: ${env.BRANCH_NAME}\n" + \
                     "Status: ${build.result}\n" + \
                     "Job: ${env.BUILD_URL} \n" + \
                     "Regression: ${env.JOB_URL}Regression_Report/\n" + \
                     "Changes: " + getChangeString(build) 
        )
    }
}

def cmake(opts, externalModules, onModules, offModules) {
    return "cmake -G Ninja -LA " +
        opts.inject("", {res, item -> res + " -D" + item.key + "=" + item.value}) + 
        (externalModules ? " -DIVW_EXTERNAL_MODULES=" + externalModules.join(";") : "" ) +
        onModules.inject("", {res, item -> res + " -D" + "IVW_MODULE_" + item + "=ON"}) +
        offModules.inject("", {res, item -> res + " -D" + "IVW_MODULE_" + item + "=OFF"}) + 
        "../inviwo"
}

def clean(params) {
    if (params['Clean Build']) {
        echo "Clean build, removing build folder"
        sh "rm -r build"
    }
}

Map defaultOptions(params) {
    return [
        "CMAKE_CXX_COMPILER_LAUNCHER" : "ccache",
        "CMAKE_BUILD_TYPE" : params['Build Type'],
        "OpenCL_LIBRARY" : "/usr/local/cuda/lib64/libOpenCL.so",
        "OpenCL_INCLUDE_DIR" : "/usr/local/cuda/include/",
        "CMAKE_PREFIX_PATH" : "/opt/Qt/5.6/gcc_64",
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

def build(opts, externalModules, onModules, offModules = []) {
    dir('build') {
        println("Options: ${opts.inject('', {res, item -> res + '\n  ' + item.key + ' = ' + item.value})}")
        println("External Modules: ${externalModules.inject('', {res, item -> res + '\n  ' + item})}")
        println("Modules On: ${onModules.inject('', {res, item -> res + '\n  ' + item})}")
        println("Modules Off: ${offModules.inject('', {res, item -> res + '\n  ' + item})}")
        log {
            sh """
                ccache -z # reset ccache statistics
                # tell ccache where the project root is
                export CPATH=`pwd`
                export CCACHE_BASEDIR=`readlink -f \${CPATH}/..`
                        
                ${cmake(opts, externalModules, onModules, offModules, indent)}

                ninja

                ccache -s # print ccache statistics
            """
        }
    }    
}

def buildStandard(params, externalModules, extraOpts, onModules, offModules = []) {
    stage('Build') {
        clean(params)
        def opts = defaultOptions(params)
        extraOpts.each {item ->
            opts[item.key] = item.value
        }
        build(opts, onModules, offModules, indent)
    }
}

return this
