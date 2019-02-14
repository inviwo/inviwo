node {
    stage('Fetch') { 
        dir('inviwo') {
            checkout scm
            sh 'git submodule sync --recursive' // needed when a submodule has a new url  
            sh 'git submodule update --init --recursive'
        }
    }

    def util = load "${env.WORKSPACE}/inviwo/tools/jenkins/util.groovy"
    if(!env.disabledProperties) properties(util.defaultProperties())
    util.printMap("Environment", env.getEnvironment())
    
    Map state = [
        env: env,
        build: currentBuild, 
        errors: [],
        display: 0,
        addLabel: {label -> 
            if (env.CHANGE_ID  && !(label in pullRequest.labels)) pullRequest.addLabels([label])
        },
        removeLabel: {label -> 
            if (env.CHANGE_ID && (label in pullRequest.labels)) pullRequest.removeLabel([label])  
        }
    ]

    util.wrap(state, "#jenkins-branch-pr") {
        util.buildStandard(
            state: state,
            modulePaths: [], 
            onModules: ["DiscreteData", "HDF5", "OpenCL", "BaseCL", "WebBrowser", "Example"],  
            offModules: ["ABufferGL"],
            opts: [:]
        )
        util.filterfiles()
        util.format(state)
        util.warn(state)
        util.unittest(state)
        util.integrationtest(state)        
        util.regression(state, ["${env.WORKSPACE}/inviwo/modules"])
        util.copyright(state)    
        util.doxygen(state)
    }
}
