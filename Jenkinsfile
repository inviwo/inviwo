
class State {
    def env
    def params
    def build
    def setLabel
    def removeLabel
    Integer display = 0 
    List errors = []
}

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
    List off = ["ABUFFERGL" , "DISCRETEDATA", "HDF5"]

    def state = new State(
        env: env,
        params: params, 
        build: currentBuild, 
        setLabel: {label -> 
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
    )

    try {
        util.buildStandard(
            state: state,
            modulePaths: modulePaths, 
            onModules: on,  
            offModules: off
        )
        util.filterfiles()
        util.format(state)
        util.warn(state)
        util.unittest(state)
        util.integrationtest(state)        
        //util.regression(state, ["${env.WORKSPACE}/inviwo/modules"])
        util.copyright(state)
        util.doxygen(state)       
        util.publish()

        state.build.result = 'SUCCESS'
    } catch (e) {
        state.build.result = 'FAILURE'
        throw e
    } finally {
        util.slack(state, "#jenkins-branch-pr")
    }
}
