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
    println "Env:" + env.getEnvironment()?.collect{"${it.key.padLeft(25)} = ${it.value}"}?.join("\n") ?: ''

    Map state = [
        env: env,
        build: currentBuild, 
        errors: [],
        display: 0,
        addLabel: {label -> 
            if (env.CHANGE_ID  && (!label in pullRequest.labels)) {
                println("Add label: ${label}")
                pullRequest.addLabels([label])
            }
        },
        removeLabel: {label -> 
            if (env.CHANGE_ID && label in pullRequest.labels) {
                println("Remove label: ${label}")
                pullRequest.removeLabel([label])
            }
        }
    ]

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

        state.build.result = state.errors.isEmpty() ? 'SUCCESS' : 'UNSTABLE'
    } catch (e) {
        state.build.result = 'FAILURE'
        throw e
    } finally {
        util.slack(state, "#jenkins-branch-pr")
        if (!state.errors.isEmpty()) {
            println "Errors in: ${state.errors.join(", ")}"
            state.build.displayName = "#${state.build.number} Failure"
            state.build.description = "Errors in: ${state.errors.join(' ')}"
        } else {
            state.build.displayName = "#${state.build.number} Success"
        }
    }
}
