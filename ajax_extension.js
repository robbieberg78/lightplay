(function(ext) {


   ext.sendall = function(op_code, callback){
      console.log("call start");
      if(0 <= op_code && op_code <= 255){
         console.log("ajax start");
            $.ajax({
                    url: 'http://localhost:8080',
                    data: {byte: op_code},
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
                   ['R', 'send byte: %n', 'sendall', 0x00],
                   ['R', 'Test: %n', 'test', 0x00],
                   ['R', 'recv byte', 'recv']
                   ],

           url: 'https://github.com/bsb20/scratch-to-serial/tree/gh-pages'
   };

   // Register the extension
   ScratchExtensions.register('Sample extension', descriptor, ext);
})({});



