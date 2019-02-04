node {
    stage('Fetch') { 
        dir('inviwo') {
            checkout scm
            sh 'git submodule sync --recursive' // needed when a submodule has a new url  
            sh 'git submodule update --init --recursive'
        }
    }
    println "Env:\n" + env.getEnvironment()?.collect{"${it.key.padLeft(25)} = ${it.value}"}?.join("\n  ") ?: ''
    def util = load "${env.WORKSPACE}/inviwo/tools/jenkins/util.groovy"      
    properties(util.defaultProperties(env))

    Map state = [
        env: [:]
        build: currentBuild, 
        errors: [],
        display: 0,
        addLabel: {label -> 
            println("Add label: ${label}")
            if (env.CHANGE_ID  && (!label in pullRequest.labels)) {
                pullRequest.addLabels([label])
            }
        },
        removeLabel: {label -> 
            println("Remove label: ${label}")
            if (env.CHANGE_ID && label in pullRequest.labels) {
                pullRequest.removeLabel([label])
            }
        }
    ]
    env.getEnvironment().each { k, v -> state.env.put(k, v) }
    
    println "Env2:\n" + state.env?.collect{"${it.key.padLeft(25)} = ${it.value}"}?.join("\n  ") ?: ''

    try {
        util.buildStandard(
            state: state,
            modulePaths: [], 
            onModules: [],  
            offModules: ["ABUFFERGL"],
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

        state.build.result = state.errors.isEmpty() ? 'SUCCESS' : 'FAILURE'
    } catch (e) {
        state.build.result = 'FAILURE'
        throw e
    } finally {
        util.slack(state, "#jenkins-branch-pr")
        if (!state.errors.isEmpty()) {
            println "Errors in: ${state.errors.join(" ")}"
        }
    }
}
