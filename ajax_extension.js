(function(ext) {


   ext.sendall = function(channel, action, callback){
      console.log("call start");
      if(0 <= channel && channel <= 9){
         console.log("ajax start");
            $.ajax({
                    url: 'http://localhost:8080',
                    data: {channel: channel, action: action },
                    success: function(data,tStatus,xhr) {
                     console.log(data);
                     console.log(tStatus);
                     callback("SUCCESS");
                    },
                    error: function(xhr, tStatus, error){
                     console.log(tStatus);
                     console.log(error);
                     callback("ERROR");
                    },
                    complete: function(xhr, tStatus){
                       console.log(tStatus);
                       callback("COMPLETE");
                    }
                    
                    }
                  );
         }
   };

   ext.test = function(arg, callback){
      
      callback(arg);
   };

   ext.recv = function(callback){
           callback("data");
   };

   var descriptor = {
           blocks: [
                   ['R', 'Turn channel %n %m.action', 'sendall', 1, On],
                   ['R', 'Test: %n', 'test', 0x00],
                   ['R', 'recv byte', 'recv']
                   ],

           url: 'https://github.com/bsb20/scratch-to-serial/tree/gh-pages',
           menus: {
               action: ['On', 'Off']
           }
   };

   // Register the extension
   ScratchExtensions.register('Sample extension', descriptor, ext);
})({});



