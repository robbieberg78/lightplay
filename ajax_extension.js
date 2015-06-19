(function(ext) {


   ext.sendall = function(op_code){
      if(0 <= raw_bye && op_code <= 255){
            $.ajax({
                    url: 'localhost:8080',
                    data: {byte: op_code},
                    success: function(data,tStatus,xhr) {
                     console.log(data);
                     console.log(tStatus);
                    },
                    error: function(xhr, tStatus, error){
                     console.log(tStatus);
                     console.log(error);
                    }
                    
                    }
                  );
                  
            return "one";
         }
         return "two";
   };

   ext.test = function(arg){
      
      return "TEST";
   };

   ext.recv = function(callback){
           callback("data");
   };

   var descriptor = {
           blocks: [
                   ['r', 'send byte: %n', 'sendall', 0x00],
                   ['r', 'Test: %n', 'test', 0x00],
                   ['R', 'recv byte', 'recv']
                   ],

           url: 'https://github.com/bsb20/scratch-to-serial/tree/gh-pages'
   };

   // Register the extension
   ScratchExtensions.register('Sample extension', descriptor, ext);
})({});



