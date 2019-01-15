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
        def modulePaths = []
        def on = []
        def off = ["ABUFFERGL" , "DISCRETEDATA", "HDF5"]

        util.buildStandard(
            params : params, 
            modulePaths : modulePaths, 
            onModules : on,  
            offModules : off)

        util.warn()
        util.unittest()
        util.integrationtest()        
        //util.regression(currentBuild, env, ["${env.WORKSPACE}/inviwo/modules"])
        util.copyright()
        util.doxygen()       
        util.publish()
        
        if (env.CHANGE_ID) {
            if (fileExists("commentid.txt")) {
                id = readFile file: "commentid.txt"
                pullRequest.editComment(it, 'tested by jenkins again')
            } else {
                def comment = pullRequest.comment('tested by jenkins')
                writeFile file: "commentid.txt", text: comment.id
            }
        }

        currentBuild.result = 'SUCCESS'
    } catch (e) {
        currentBuild.result = 'FAILURE'
        throw e
    } finally {
        util.slack(currentBuild, env, "#jenkins-branch-pr")
    }
}
