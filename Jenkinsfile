node {
    properties([
        parameters([
            booleanParam(
                defaultValue: false, 
                description: 'Do a clean build', 
                name: 'Clean Build'
            ),
            choice(
                choices: "Release\nDebug\nMinSizeRel\nRelWithDebInfo\n", 
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
                        set +x
                        cmake -G \"Unix Makefiles\" -LA \
                                -DCMAKE_BUILD_TYPE=${params['Build Type']} \
                                -DOpenCL_LIBRARY=/usr/local/cuda/lib64/libOpenCL.so  \
                                -DOpenCL_INCLUDE_DIR=/usr/local/cuda/include/ \
                                -DCMAKE_PREFIX_PATH=/opt/Qt/5.6/gcc_64 \
                                -DIVW_CMAKE_DEBUG=ON \
                                -DBUILD_SHARED_LIBS=ON \
                            ../inviwo
 
                        make -j 8
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
            publishHTML([
                allowMissing: false, 
                alwaysLinkToLastBuild: false, 
                keepAll: false, 
                reportDir: 'regress', 
                reportFiles: 'report.html', 
                reportName: 'Regression Report'])
        }
        currentBuild.result = 'SUCCESS'
    } catch (e) {
        currentBuild.result = 'FAILURE'
        throw e
    } finally {
        echo "result: ${currentBuild.result}"
        def res2color = ['SUCCESS' : 'good', 'UNSTABLE' : 'warning' , 'FAILURE' : 'danger' ]
        def color = res2color.containsKey(currentBuild.result) ? res2color[currentBuild.result] : 'warning'
        slackSend(color:color, message: "Inviwo branch: ${env.BRANCH_NAME}\n" + \
            "Status: ${currentBuild.result}\n" + \
            "Job: ${env.BUILD_URL} \n" + \
            "Regression: ${env.JOB_URL}Regression_Report/"
        )
    }
}