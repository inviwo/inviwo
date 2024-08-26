node {
    stage('Fetch') { 
        dir('inviwo') {
            scmVars = checkout scm
        }
    }

    def util = load "${env.WORKSPACE}/inviwo/tools/jenkins/util.groovy"
    util.config(this)
    
    util.wrap(this, "#jenkins-branch-pr") {
        util.touchwarn()
        util.format(this, "${env.WORKSPACE}/inviwo")
        util.buildStandard(
            state: this,
            modulePaths: [], 
            onModules: ["DiscreteData", "HDF5", "OpenCL", "BaseCL",
                        "WebBrowser", "Example"],
            offModules: ["ABufferGL"],
            opts: [:],
            preset: "ninja-developer"
        )
        util.warn(this)
        util.unittest(this)
        util.integrationtest(this)
        util.regression(this, ["${env.WORKSPACE}/inviwo/modules"])
        util.copyright(this)
        util.doxygen(this)
    }
}
