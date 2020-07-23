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
    stage('ConfigureAndBuild'){
      matrix {
        axes {
          axis {
            name 'PLATFORM'
            values 'linux64', 'linux32', 'windows64', 'windows32'
          }
          axis {
            name 'BUILDTYPE'
            values 'Release', 'Debug'
          }
        }
        stages {
          stage('Clean'){
            steps {
              sh 'git clean -fdx'
            }
          }
          stage('linux64'){
            when {
              expression {
                PLATFORM == 'linux64'
              }
            }
            steps {
              sh 'cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="${BUILDTYPE}" .'
            }
          }
          stage('linux32'){
            when {
              expression {
                PLATFORM == 'linux32'
              }
            }
            steps {
              sh 'CFLAGS="-m32" CXXFLAGS="-m32" /bin/setarch i686 cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="${BUILDTYPE}" .'
            }
          }
          stage('windows64'){
            when {
              expression {
                PLATFORM == 'windows64'
              }
            }
            steps {
              sh 'mingw64-cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="${BUILDTYPE}" .'
            }
          }
          stage('windows32'){
            when {
              expression {
                PLATFORM == 'windows32'
              }
            }
            steps {
              sh 'mingw32-cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="${BUILDTYPE}" .'
            }
          }
          stage('Build'){
            steps {
              sh 'make package'
            }
            post {
              always {
                archiveArtifacts artifacts: 'sepiola-*.*', onlyIfSuccessful: true
              }
            }
          }
        }
      }
    }
  }
}
