@NonCPS
def getChangeString() {
    MAX_MSG_LEN = 100
    def changeString = ""

    echo "Gathering SCM changes"
    def changeLogSets = currentBuild.rawBuild.changeSets
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

node {
    properties([
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
    ])
    try {
        stage('Fetch') { 
            echo "Building inviwo Running ${env.BUILD_ID} on ${env.JENKINS_URL}"
            dir('inviwo') {
                checkout scm
                sh 'git submodule update --init'
            }
        }
        stage('Build') {
            if (params['Clean Build']) {
                echo "Clean build, removing build folder"
                sh "rm -r build"
            }
            dir('build') {
                withEnv(['TERM=xterm', 'CC=/usr/bin/gcc-5', 'CXX=/usr/bin/g++-5']) {
                    sh """
                        cmake -G \"Unix Makefiles\" -LA \
                              -DCMAKE_BUILD_TYPE=${params['Build Type']} \
                              -DOpenCL_LIBRARY=/usr/local/cuda/lib64/libOpenCL.so  \
                              -DOpenCL_INCLUDE_DIR=/usr/local/cuda/include/ \
                              -DCMAKE_PREFIX_PATH=/opt/Qt/5.6/gcc_64 \
                              -DIVW_CMAKE_DEBUG=ON \
                              -DBUILD_SHARED_LIBS=ON \
                              -DIVW_MODULE_GLFW=ON \
                              -DIVW_TINY_GLFW_APPLICATION=ON \
                              -DIVW_TINY_QT_APPLICATION=ON \
                              -DIVW_MODULE_ABUFFERGL=ON \
                              -DIVW_MODULE_ANIMATION=ON \
                              -DIVW_MODULE_ANIMATIONQT=ON \
                              ../inviwo
 
                        make -j 6
                    """
                }
            }
        }
        stage('Regression') {
            dir('regress') {
                sh """
                    export DISPLAY=:0
                    python3 ../inviwo/tools/regression.py \
                            --inviwo ../build/bin/inviwo \
                            --header ${env.JENKINS_HOME}/inviwo-config/header.html \
                            --output . \
                            --repos ../inviwo
                """
            }
        }
        currentBuild.result = 'SUCCESS'
    } catch (e) {
        currentBuild.result = 'FAILURE'
        throw e
    } finally {
        stage('Publish') {
            publishHTML([
                allowMissing: true, 
                alwaysLinkToLastBuild: true, 
                keepAll: false, 
                reportDir: 'regress', 
                reportFiles: 'report.html', 
                reportName: 'Regression Report'
            ])

            echo "result: ${currentBuild.result}"
            def res2color = ['SUCCESS' : 'good', 'UNSTABLE' : 'warning' , 'FAILURE' : 'danger' ]
            def color = res2color.containsKey(currentBuild.result) ? res2color[currentBuild.result] : 'warning'
            slackSend(
                color: color, 
                message: "Inviwo branch: ${env.BRANCH_NAME}\n" + \
                         "Status: ${currentBuild.result}\n" + \
                         "Job: ${env.BUILD_URL} \n" + \
                         "Regression: ${env.JOB_URL}Regression_Report/\n" + \
                         "Changes: " + getChangeString() 
            )
        }
    }
}