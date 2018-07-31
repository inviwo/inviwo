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

def nicelog(env = [], fun) {
    withEnv(['TERM=xterm'] + env) {
        ansiColor {
            timestamps {
                fun()
            }
        }
    }
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
                sh 'git submodule sync' // needed when a submodule has a new url  
                sh 'git submodule update --init'
            }
        }

        stage('Build') {
            if (params['Clean Build']) {
                echo "Clean build, removing build folder"
                sh "rm -r build"
            }
            dir('build') {
                nicelog(['CC=/usr/bin/gcc-5', 'CXX=/usr/bin/g++-5']) {
                    sh """
                        ccache -z # reset ccache statistics
                        # tell ccache where the project root is
                        export CPATH=`pwd`
                        export CCACHE_BASEDIR=`readlink -f \${CPATH}/..`
                        
                        cmake -G \"Ninja\" -LA \
                              -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
                              -DCMAKE_BUILD_TYPE=${params['Build Type']} \
                              -DOpenCL_LIBRARY=/usr/local/cuda/lib64/libOpenCL.so  \
                              -DOpenCL_INCLUDE_DIR=/usr/local/cuda/include/ \
                              -DCMAKE_PREFIX_PATH=/opt/Qt/5.6/gcc_64 \
                              -DIVW_CMAKE_DEBUG=ON \
                              -DIVW_DOXYGEN_PROJECT=ON \
                              -DBUILD_SHARED_LIBS=ON \
                              -DIVW_MODULE_GLFW=ON \
                              -DIVW_TINY_GLFW_APPLICATION=ON \
                              -DIVW_TINY_QT_APPLICATION=ON \
                              -DIVW_MODULE_ABUFFERGL=ON \
                              -DIVW_MODULE_ANIMATION=ON \
                              -DIVW_MODULE_ANIMATIONQT=ON \
                              -DIVW_MODULE_PLOTTING=ON \
                              -DIVW_MODULE_PLOTTINGGL=ON \
                              -DIVW_MODULE_DEMO=ON \
                              -DIVW_MODULE_DEMOQT=ON \
                              -DIVW_MODULE_POSTPROCESSING=ON \
                              -DIVW_MODULE_USERINTERFACEGL=ON \
                              -DIVW_MODULE_HDF5=ON \
                              -DIVW_UNITTESTS=ON \
                              -DIVW_UNITTESTS_RUN_ON_BUILD=OFF \
                              -DIVW_INTEGRATION_TESTS=ON \
                              -DIVW_RUNTIME_MODULE_LOADING=ON \
                              ../inviwo

                        ninja

                        ccache -s # print ccache statistics
                    """
                }
            }
        }

        stage('Unit tests') {
            dir('build/bin') {
                nicelog {
                    sh '''
                        export DISPLAY=:0
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

        stage('Integration tests') {
            dir('build/bin') {
                nicelog {
                    sh '''
                        export DISPLAY=:0
                        ./inviwo-integrationtests
                    '''
                }
            }
        }

        stage('Copyright check') {
            dir('inviwo') {
            nicelog {
                sh '''
                python3 tools/refactoring/check-copyright.py .
                '''
            }

            }

        }
        
        stage('Doxygen') {
            dir('build') {
                nicelog {
                    sh '''
                        export DISPLAY=:0
                        ninja DOXY-ALL
                    '''
                }
            }
        }
        
        try {
            stage('Regression tests') {
                dir('regress') {
                    nicelog {
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
            }
        } catch (e) {
            // Mark as unstable, if we mark as failed, the report will not be published.
            currentBuild.result = 'UNSTABLE'
        }
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
        currentBuild.result = 'SUCCESS'
    } catch (e) {
        currentBuild.result = 'FAILURE'
        throw e
    } finally {
        stage('Slack') {
            echo "result: ${currentBuild.result}"
            def res2color = ['SUCCESS' : 'good', 'UNSTABLE' : 'warning' , 'FAILURE' : 'danger' ]
            def color = res2color.containsKey(currentBuild.result) ? res2color[currentBuild.result] : 'warning'
            slackSend(
                color: color, 
                channel: "#jenkins-branch-pr", 
                message: "Inviwo branch: ${env.BRANCH_NAME}\n" + \
                         "Status: ${currentBuild.result}\n" + \
                         "Job: ${env.BUILD_URL} \n" + \
                         "Regression: ${env.JOB_URL}Regression_Report/\n" + \
                         "Changes: " + getChangeString() 
            )
        }
    }
}
