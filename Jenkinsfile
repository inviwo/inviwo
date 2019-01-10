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
    def display = 0       

    try {
        def external = []
        def extraOn = []
        def off = ["ABUFFERGL" , "DISCRETEDATA", "GLFW", "HDF5"]

        util.buildStandard(params : params, external : external, onModules : extraOn,  offModules : off)
        util.warn()
        util.unittest()
        util.integrationtest()        
        util.regression(currentBuild, env)
        util.copyright()
        util.doxygen()       
        util.publish()

        currentBuild.result = 'SUCCESS'
    } catch (e) {
        currentBuild.result = 'FAILURE'
        throw e
    } finally {
        util.slack(currentBuild, env, "#jenkins-branch-pr")
    }
}
