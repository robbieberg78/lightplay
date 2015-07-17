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

   function send_to_channel(channel, action, callback) {
      if (descriptor.menus.channels.indexOf(channel) >= 0) {
         channel = descriptor.menus.channels.indexOf(channel);
      }
      send_to(channel, action, callback);
   }

   function send_to_sensor(sensor, action, callback) {
      if (descriptor.menus.sensors.indexOf(sensor) >= 0) {
         sensor = descriptor.menus.sensors.indexOf(sensor) + 1;
      }
      send_to(sensor, action, callback);
   }

   ext.send_on = function(channel, callback) {
      send_to_channel(channel, "On", callback);
   };

   ext.send_off = function(channel, callback) {
      send_to_channel(channel, "Off", callback);
   };

   ext.send_toggle = function(channel, callback) {
      send_to_channel(channel, "Toggle", callback);
   };



   ext.send_all_on = function(callback) {
      send_to_channel(0, "On", callback);
   };

   ext.send_all_off = function(callback) {
      send_to_channel(0, "Off", callback);
   };

   ext.send_rev = function(channel, callback) {
      send_to_channel(channel, "Rev", callback);
   };

   function register(sensor, callback) {
      send_to_sensor(sensor, "Register", callback);
   }

   function sensor_manager() {
      this.sensors = {};
      this.register_and_poll = function(sensor) {
         if (!(sensor in this.sensors)) {
            this.sensors[sensor] = {
               state: false,
               listeners: 1
            };
            register(sensor, get_callback(sensor));
            return false;
         }
         var sensor_data = this.sensors[sensor];
         if (sensor_data.state) {
            if (sensor_data.listeners === 0) {
               sensor_data.listeners += 1;
               sensor_data.state = false;
               register(sensor, get_callback(sensor));
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

   ext.poll_for_on = function(sensor) {
      if (manager.register_and_poll(sensor)) {
         console.log(manager.sensors[sensor].data);
         var result = manager.sensors[sensor].data === "True";
         manager.sensors[sensor].data = "";
         return result;
      }
   };

   ext.poll = function(sensor, callback) {
      send_to_sensor(sensor, "Poll", callback);
   };

   ext.set_power = function(channel, level, callback) {
      send_to_channel(channel, level, callback);
   };

   ext.fade_in = function(channel, callback) {
      send_to_channel(channel, "FadeIn", callback);
   };

   ext.fade_out = function(channel, callback) {
      send_to_channel(channel, "FadeOut", callback);
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
         ['w', 'Turn on  %m.channels', 'send_on', "A"],
         ['w', 'Turn off  %m.channels', 'send_off', "A"],
         ['w', 'Toggle  %m.channelsalt', 'send_toggle', "A"],
         ['w', 'Turn on all', 'send_all_on'],
         ['w', 'Turn off all', 'send_all_off'],
         ['w', 'Reverse motor  %m.channels', 'send_rev', "A"],
         ['w', 'Set light %m.channels to %m.power power', 'set_power', "A", 'High'],
         ['w', 'Fade in light %m.channelsalt', 'fade_in', "A"],
         ['w', 'Fade out light %m.channelsalt', 'fade_out', "A"],
         ['w', 'Set rainbow light color to %m.colors', 'set_color', "Red"],
         ['w', 'Fade rainbow light color to %m.colors', 'fade_color', "Red"],
         ['w', 'Turn rainbow light off', 'bt_off'],
         ['h', 'When sensor %m.sensors clips are connected', 'poll_for_on', "Sensor A"],
         ['R', 'Value of %m.sensors', 'poll', "Sensor A"]
      ],

      url: 'https://github.com/bsb20/scratch-to-serial/tree/gh-pages',
      menus: {
         action: ['On', 'Off'],
         power: ['Low', 'Med', 'High'],
         colors: ['White', 'Blue', 'Teal', 'Green', 'Yellow', 'Orange', 'Red', 'Pink', 'Purple', 'Surprise'],
         sensors: ["Sensor A", "Sensor B"],
         channels: ["All", "A", "B", "C", "D"],
         channelsalt: ["A", "B", "C", "D"]
      }
   };

   // Register the extension..
   ScratchExtensions.register('Sample extension', descriptor, ext);
})({});
