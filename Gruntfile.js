module.exports = function (grunt){

  // Project configuration.
  src_files = ['src/*.cc'];
  grunt.initConfig({
    pkg: grunt.file.readJSON('package.json'),
    watch:{
      configFiles:{ 
        files:{ 
          [ 'Gruntfile.coffee' ]
        }
        options:{ 
          reload: true        
        }
      scripts:{
        files:['src/*.cc', 'src/*.h']
        tasks:['gyp']
      }
    },

    gyp:{
      fusejs:{ 
        options:{
          debug: false
        },
        command: 'rebuild'
      }
    }

    

  
  );

  grunt.loadNpmTasks('grunt-node-gyp');
  grunt.loadNpmTasks('grunt-contrib-watch');
  # grunt.loadNpmTasks('grunt-mocha-test');
  # grunt.loadNpmTasks('grunt-contrib-copy');
  # grunt.loadNpmTasks('grunt-node-gyp');


  # Default task(s).
  grunt.registerTask('default', [ 'gyp']);
