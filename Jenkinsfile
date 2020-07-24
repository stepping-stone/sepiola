pipeline {
  agent {
    docker {
      image 'localhost:5000/sst/build-sepiola:latest'
      registryUrl 'http://localhost:5000'
      reuseNode true
    }
  }
  stages {
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
          stage('Checkout'){
            steps {
              ws("${env.WORKSPACE}/${PLATFORM}-${BUILDTYPE}"){
                checkout scm
              }
            }
          }
          stage('Clean'){
            steps {
              ws("${env.WORKSPACE}/${PLATFORM}-${BUILDTYPE}"){
                sh 'git clean -fdx'
              }
            }
          }
          stage('linux64'){
            when {
              expression {
                PLATFORM == 'linux64'
              }
            }
            steps {
              ws("${env.WORKSPACE}/${PLATFORM}-${BUILDTYPE}"){
                sh 'cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="${BUILDTYPE}" .'
              }
            }
          }
          stage('linux32'){
            when {
              expression {
                PLATFORM == 'linux32'
              }
            }
            steps {
              ws("${env.WORKSPACE}/${PLATFORM}-${BUILDTYPE}"){
                sh 'CFLAGS="-m32" CXXFLAGS="-m32" /bin/setarch i686 cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="${BUILDTYPE}" -DQT_TRANSLATIONS_PATH="/usr/share/qt5/translations" .'
              }
            }
          }
          stage('windows64'){
            when {
              expression {
                PLATFORM == 'windows64'
              }
            }
            steps {
              ws("${env.WORKSPACE}/${PLATFORM}-${BUILDTYPE}"){
                sh 'mingw64-cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="${BUILDTYPE}" -DQT_TRANSLATIONS_PATH="/usr/share/qt5/translations" .'
              }
            }
          }
          stage('windows32'){
            when {
              expression {
                PLATFORM == 'windows32'
              }
            }
            steps {
              ws("${env.WORKSPACE}/${PLATFORM}-${BUILDTYPE}"){
                sh 'mingw32-cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="${BUILDTYPE}" -DQT_TRANSLATIONS_PATH="/usr/share/qt5/translations" .'
              }
            }
          }
          stage('Build'){
            steps {
              ws("${env.WORKSPACE}/${PLATFORM}-${BUILDTYPE}"){
                sh 'make package'
              }
            }
            post {
              always {
                ws("${env.WORKSPACE}/${PLATFORM}-${BUILDTYPE}"){
                  archiveArtifacts artifacts: 'sepiola-*.*', onlyIfSuccessful: true
                }
              }
            }
          }
        }
      }
    }
  }
}
