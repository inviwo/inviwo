node {
    stage('Fetch') { 
        dir('inviwo') {
            scmVars = checkout scm
            sh 'git submodule sync --recursive' // needed when a submodule has a new url  
            sh 'git submodule update --init --recursive'
        }
    }

    def util = load "${env.WORKSPACE}/inviwo/tools/jenkins/util.groovy"
    if (!env.disabledProperties) properties(util.defaultProperties())
    util.printMap("Environment", env.getEnvironment())
    
    Map state = [
        env: env,
        build: currentBuild, 
        errors: [],
        display: 0,
        addLabel: {String l -> util.ifdef({pullRequest})?.addLabels([l])},
        removeLabel: {String l -> try { util.ifdef({pullRequest})?.removeLabel(l) } catch(e) {}}
    ]

    util.repl(this, scmVars)
    util.repl(this, state)
    util.repl(this, util.ifdef({pullRequest}))

    util.wrap(state, "#jenkins-branch-pr") {
        util.touchwarn()
        util.buildStandard(
            state: state,
            modulePaths: [], 
            onModules: ["DiscreteData", "HDF5", "OpenCL", "BaseCL", "WebBrowser", "Example"],  
            offModules: ["ABufferGL"],
            opts: [:]
        )
        util.format(state, ["${env.WORKSPACE}/inviwo"])
        util.warn(state)
        util.unittest(state)
        util.integrationtest(state)        
        //util.regression(state, ["${env.WORKSPACE}/inviwo/modules"])
        util.copyright(state)    
        util.doxygen(state)
    }
}
