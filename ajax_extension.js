(function(ext) {

   var PLUGIN_URL = 'http://localhost:8000';

   ext._getStatus = function() {
      return {
         status: 2,
         msg: 'Ready'
      };
   };


   function sendall(channel, action, callback) {
      console.log("call start");
      if (0 <= channel && channel <= 9) {
         console.log("ajax start");
         $.ajax({
            url: PLUGIN_URL,
            data: {
               channel: channel,
               action: action
            },
            success: function(data, tStatus, xhr) {
               console.log(data);
               console.log(tStatus);
               callback(data);
            },
            error: function(xhr, tStatus, error) {
               console.log(tStatus);
               console.log(error);
               callback("ERROR");
            },
            complete: function(xhr, tStatus) {
               console.log(tStatus);
            }

         });
      }
   }

   ext.send_on = function(channel, callback) {
      sendall(channel, "On", callback);
   };

   ext.send_off = function(channel, callback) {
      sendall(channel, "Off", callback);
   };

   ext.send_all_on = function(callback) {
      sendall(0, "On", callback);
   };

   ext.send_all_off = function(callback) {
      sendall(0, "Off", callback);
   };

   ext.send_rev = function(channel, callback) {
      sendall(channel, "Rev", callback);
   };

   function register(channel, callback) {
      sendall(channel, "Register", callback);
   }

   function sensor_manager() {
      this.sensors = {};
      this.register_and_poll = function(channel) {
         if (!(channel in this.sensors)) {
            this.sensors[channel] = {
               state: false,
               listeners: 0
            };
         }
         var sensor = this.sensors[channel];
         if (sensor.state) {
            sensor.listeners += 1;
            sensor.state = false;
            register(channel, get_callback(channel));
            return true;
         }
         return false;
      };
   }

   var manager = new sensor_manager();

   function get_callback(channel) {
      return function set_states(data) {
         manager.sensors[channel].state = true;
         manager.sensors[channel].listeners -= 1;
      };
   }




   ext.register_and_poll = function(channel) {
      return manager.register_and_poll(channel);
   };

   ext.poll = function(channel, callback) {
      sendall(channel, "Poll", callback);
   };

   var descriptor = {
      blocks: [
         ['w', 'Turn  %n on', 'send_on', 1],
         ['w', 'Turn  %n off', 'send_off', 1],
         ['w', 'Turn all on', 'send_all_on'],
         ['w', 'Turn all off', 'send_all_off'],
         ['w', 'Reverse  %n', 'send_rev', 1],
         ['h', 'Wait until sensor %n is pushed', 'register_and_poll', 8],
         ['R', 'Sensor %n\'s value', 'poll', 8]
      ],

      url: 'https://github.com/bsb20/scratch-to-serial/tree/gh-pages',
      menus: {
         action: ['On', 'Off']
      }
   };

   // Register the extension..
   ScratchExtensions.register('Sample extension', descriptor, ext);
})({});
