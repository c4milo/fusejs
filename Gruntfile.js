module.exports = function (grunt){

  // Project configuration.

  grunt.initConfig({
    pkg: grunt.file.readJSON('package.json'),
    watch:{
      configFiles:{ 
        files: [ 'Gruntfile.js' ],
        options:{ 
          reload: true        
        }
      },
      scripts:{
          files:['src/*.cc', 'src/*.h', 'binding.gyp'],
          tasks:['gyp', 'mochaTest']
      },
      example_test:{        
          files:['test/test_example.js', 'examples/example.js'],
          tasks:['mochaTest:example']
      },
      loopback_test:{
        files: ['test/test_loopback.js', 'examples/loopback.js'],
        tasks: ['mochaTest:loopback']
      }
    },

    gyp:{
      fusejs:{ 
        options:{
          debug: false
        },
        command: 'rebuild'
      }
    },
    
    mochaTest: {
      example: {
        options: {
          reporter: 'spec',
          quiet: false, 
          clearRequireCache: true,
          ui: 'bdd'
        },
        src: ['test/test_example.js']
      },
      loopback:{
        options: {
          reporter: 'spec',
          quiet: false, 
          clearRequireCache: true,
          ui: 'bdd',
          timeout: 5000
        },
        src: ['test/test_loopback.js']        
      }
    }

  }
  );

  grunt.loadNpmTasks('grunt-node-gyp');
  grunt.loadNpmTasks('grunt-contrib-watch');
  grunt.loadNpmTasks('grunt-mocha-test');
  //grunt.loadNpmTasks('grunt-contrib-copy');
  

  //Default task(s).
  grunt.registerTask('default', [ 'gyp']);

}