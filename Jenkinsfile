pipeline {
  agent {
    docker {
      image 'localhost:5000/sst/build-sepiola:latest'
      registryUrl 'http://localhost:5000'
      reuseNode true
    }
  }
  stages {
    stage('Checkout'){
      steps {
        checkout scm
      }
    }
    stage('Build'){
      steps {
        sh 'cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="Release" .'
        sh 'make package'
      }
      post {
        always {
          archiveArtifacts artifacts: 'sepiola*.sh', onlyIfSuccessful: true
        }
      }
    }
  }
}
