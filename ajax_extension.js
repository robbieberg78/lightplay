(function(ext) {

   var PLUGIN_URL = 'http://localhost:8000';

   ext._getStatus = function() {
      return {
         status: 2,
         msg: 'Ready'
      };
   };


   function send_obj(data_obj, callback) {
      console.log("sending object");
      $.ajax({
         url: PLUGIN_URL,
         data: data_obj,
         success: function(data, tStatus, xhr) {
            console.log(data + " " + ":: DATA");
            callback(data);
         },
         error: function(xhr, tStatus, error) {
            callback("ERROR");
         },
         complete: function(xhr, tStatus) {
            console.log(tStatus);
         }

      });
   }


   function send_to(channel, action, callback) {
      console.log(action + " start");
      if (0 <= channel && channel <= 9) {
         $.ajax({
            url: PLUGIN_URL,
            data: {
               channel: channel,
               action: action
            },
            success: function(data, tStatus, xhr) {
               console.log(action + " " + tStatus);
               console.log(data + " " + ":: DATA");
               callback(data);
            },
            error: function(xhr, tStatus, error) {
               console.log(action + " " + tStatus);
               callback("ERROR");
            },
            complete: function(xhr, tStatus) {
               console.log(action + " " + tStatus);
            }

         });
      }
   }

   ext.send_on = function(channel, callback) {
      send_to(channel, "On", callback);
   };

   ext.send_off = function(channel, callback) {
      send_to(channel, "Off", callback);
   };

   ext.send_all_on = function(callback) {
      send_to(0, "On", callback);
   };

   ext.send_all_off = function(callback) {
      send_to(0, "Off", callback);
   };

   ext.send_rev = function(channel, callback) {
      send_to(channel, "Rev", callback);
   };

   function register(channel, callback) {
      send_to(channel, "Register", callback);
   }

   function sensor_manager() {
      this.sensors = {};
      this.register_and_poll = function(channel) {
         if (!(channel in this.sensors)) {
            this.sensors[channel] = {
               state: false,
               listeners: 1
            };
            register(channel, get_callback(channel));
            return false;
         }
         var sensor = this.sensors[channel];
         if (sensor.state) {
            if (sensor.listeners === 0) {
               sensor.listeners += 1;
               sensor.state = false;
               register(channel, get_callback(channel));
            }
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
         manager.sensors[channel].data = data;
      };
   }

   ext.poll_for_on = function(channel) {
      if (manager.register_and_poll(channel)) {
         console.log(manager.sensors[channel].data);
         var result = manager.sensors[channel].data === "True";
         manager.sensors[channel].data = "";
         return result;
      }
   };

   ext.poll = function(channel, callback) {
      send_to(channel, "Poll", callback);
   };

   ext.set_power = function(channel, level, callback) {
      send_to(channel, level, callback);
   };

   ext.fade_in = function(channel, callback) {
      send_to(channel, "FadeIn", callback);
   };

   ext.fade_out = function(channel, callback) {
      send_to(channel, "FadeOut", callback);
   };

   ext.set_color = function(color, callback) {
      var color_code = descriptor.menus.colors.indexOf(color) + 1;
      data_obj = {
         action: "Set",
         color: color_code
      };
      send_obj(data_obj, callback);
   };

   ext.fade_color = function(color, callback) {
      var color_code = descriptor.menus.colors.indexOf(color) + 1;
      data_obj = {
         action: "FadeTo",
         color: color_code
      };
      send_obj(data_obj, callback);
   };

   ext.bt_off = function(callback) {
      data_obj = {
         action: "BtOff"
      };
      send_obj(data_obj, callback);
   };




   var descriptor = {
      blocks: [
         ['w', 'Turn  %n on', 'send_on', 1],
         ['w', 'Turn  %n off', 'send_off', 1],
         ['w', 'Turn all on', 'send_all_on'],
         ['w', 'Turn all off', 'send_all_off'],
         ['w', 'Reverse  %n', 'send_rev', 1],
         ['w', 'Set channel %n to %m.power power', 'set_power', 1, 'High'],
         ['w', 'Fade in channel %n', 'fade_in', 1],
         ['w', 'Fade out channel %n', 'fade_out', 1],
         ['w', 'Set light color to %m.colors', 'set_color', "Red"],
         ['w', 'Fade light color to %m.colors', 'fade_color', "Red"],
         ['w', 'Turn colored light off', 'bt_off'],
         ['h', 'When sensor %n clips become connected', 'poll_for_on', 1],
         ['R', 'Sensor %n\'s value', 'poll', 1]
      ],

      url: 'https://github.com/bsb20/scratch-to-serial/tree/gh-pages',
      menus: {
         action: ['On', 'Off'],
         power: ['Low', 'Med', 'High'],
         colors: ['White', 'Blue', 'Teal', 'Green', 'Yellow', 'Orange', 'Red', 'Pink', 'Purple', 'Surprise']
      }
   };

   // Register the extension..
   ScratchExtensions.register('Sample extension', descriptor, ext);
})({});
