node {
    stage('Fetch') { 
        dir('inviwo') {
            checkout scm
            sh 'git submodule sync --recursive' // needed when a submodule has a new url  
            sh 'git submodule update --init --recursive'
        }
    }
    def rootDir = pwd()
    def util = load "${rootDir}/inviwo/tools/jenkins/util.groovy"      
    properties(util.defaultProperties())

    List modulePaths = []
    List on = []
    List off = ["ABUFFERGL"]

    Map state = [
        env: env,
        params: params, 
        build: currentBuild, 
        errors: [],
        display: 0,
        addLabel: {label -> 
            if (env.CHANGE_ID) {
                if (! label in pullRequest.labels) {
                    pullRequest.addLabels([label])
                }
            }
        },
        removeLabel: {label -> 
            if (env.CHANGE_ID) {
                if (label in pullRequest.labels) {
                    pullRequest.removeLabel([label])
                }
            }
        }
    ]

    try {
        def opts = (params.cxx ? ['CMAKE_CXX_COMPILER' : params.cxx] : [:]) + 
            (params.c ? ['CMAKE_C_COMPILER' : params.c])


        util.buildStandard(
            state: state,
            modulePaths: modulePaths, 
            onModules: on,  
            offModules: off,
            opts: opts
        )
        util.filterfiles()
        util.format(state)
        util.warn(state)
        util.unittest(state)
        util.integrationtest(state)        
        util.regression(state, ["${env.WORKSPACE}/inviwo/modules"])
        util.copyright(state)    
        util.doxygen(state)

        state.build.result = 'SUCCESS'
    } catch (e) {
        state.build.result = 'FAILURE'
        throw e
    } finally {
        util.slack(state, "#jenkins-branch-pr")
    }
}
