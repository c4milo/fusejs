module.exports = function (grunt){

  // Project configuration.

  grunt.initConfig({
    pkg: grunt.file.readJSON('package.json'),
    watch:{
      configFiles:{ 
        files: [ 'Gruntfile.js' ],
        options:{ 
          reload: true        
        },
        tasks:['gyp', 'mochaTest']
      },
      scripts:{
          files:['src/*.cc', 'src/*.h', 'binding.gyp'],
          tasks:['gyp', 'mochaTest']
      },
      tests:{        
          files:['test/*.js', 'examples/example.js'],
          tasks:['mochaTest']
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
      test: {
        options: {
          reporter: 'spec',
          quiet: false, 
          clearRequireCache: false // Optionally clear the require cache before running tests (defaults to false)
        },
        src: ['test/*.js']
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