(function(ext) {

    var PLUGIN_URL = 'http://localhost:8000';
    var CHANNELS = ["all lights", "light 1", "light 2", "light 3", "motor", "Sensor A", "Sensor B"];
    var SENSORS = ["Light", "Clip"];
    var LEVELS = ["Low", "Med", "High"];
    var FADE_TIMEOUTS = [10100, 5100, 1100];
    var FADE_SPEEDS = ["FadeSlow", "FadeFaster", "FadeFastest"];

    
    ext.fade_speed = 1000;

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
        if (0 <= channel && channel <= 9 || channel == "motor") {
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
        if (CHANNELS.indexOf(channel) >= 0) {
            channel = CHANNELS.indexOf(channel);
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

    ext.send_motor_on = function(callback) {
        send_to_channel("motor", "On", callback);
    };

    ext.send_motor_off = function(callback) {
        send_to_channel("motor", "Off", callback);
    };

    ext.send_motor_toggle = function(callback) {
        send_to_channel("motor", "Toggle", callback);
    };

    ext.set_speed = function(level, callback) {
        idx = descriptor.menus.speeds.indexOf(level);
        if (idx >= 0) {
            send_to_channel("motor", LEVELS[idx], callback);
        } else {
            console.log("invalid speed");
        }
    };

    ext.send_motor_rev = function(callback) {
        send_to_channel("motor", "Rev", callback);
    };

    ext.send_all_on = function(callback) {
        send_to_channel(0, "On", callback);
    };

    ext.send_all_off = function(callback) {
        send_to_channel(0, "Off", callback);
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

    ext.poll_sensor = function(sensor) {
        var sensorID = descriptor.menus.sensors.indexOf(sensor) / 2;
        var action = descriptor.menus.sensors.indexOf(sensor) % 2;
        if (action) {
            return poll_for_off(sensorID);
        } else {
            return poll_for_on(sensorID);
        }
    };

    function poll_for_on(sensor) {
        if (SENSORS.indexOf(sensor) >= 0) {
            sensor = SENSORS.indexOf(sensor);
        }
        if (manager.register_and_poll(sensor) && manager.sensors[sensor].data === "True") {
            console.log(manager.sensors[sensor].data);
            manager.sensors[sensor].data = "";
            return true;
        }
        return false;
    }

    function poll_for_off(sensor) {
        if (SENSORS.indexOf(sensor) >= 0) {
            sensor = SENSORS.indexOF(sensor);
        }
        if (manager.register_and_poll(sensor) && manager.sensors[sensor].data === "False") {
            console.log(manager.sensors[sensor].data);
            manager.sensors[sensor].data = "";
            return true;
        }
        return false;
    }



    ext.poll = function(sensor, callback) {
        send_to_sensor(sensor, "Poll", callback);
    };

    ext.set_power = function(channel, level, callback) {
        idx = descriptor.menus.power.indexOf(level);
        if (idx >= 0) {
            send_to_channel(channel, LEVELS[idx], callback);
        } else {
            console.log("invalid power");
        }
    };

    ext.set_fade_speed = function(speed, callback) {
        idx = descriptor.menus.speeds.indexOf(speed);
        if (idx >= 0) {
            ext.fade_speed = FADE_TIMEOUTS[idx];
            send_to_channel(0, FADE_SPEEDS[idx], callback);
        } else {
            console.log("invalid speed");
        }
    };

    ext.fade_in = function(channel, callback) {
        send_to_channel(channel, "FadeIn", function(data){
            setTimeout(callback, ext.fade_speed, data);
        });
        
    };

    ext.fade_out = function(channel, callback) {
        send_to_channel(channel, "FadeOut", function(data){
            setTimeout(callback, ext.fade_speed, data);
        });
    };

    ext.set_color = function(channel, color, callback) {
        var color_code = descriptor.menus.colors.indexOf(color);
        data_obj = {
            action: "Set",
            color: color_code,
            channel: CHANNELS.indexOf(channel)
        };
        send_obj(data_obj, callback);
    };

    ext.fade_color = function(channel, color, callback) {
        var color_code = descriptor.menus.colors.indexOf(color);
        data_obj = {
            action: "FadeTo",
            color: color_code,
            channel: CHANNELS.indexOf(channel)
        };
        send_obj(data_obj, function(data){
            setTimeout(callback, ext.fade_speed, data);
        });
    };

    ext.bt_off = function(callback) {
        data_obj = {
            action: "BtOff"
        };
        send_obj(data_obj, callback);
    };




    var descriptor = {
        blocks: [
            // Light cmds
            ['w', 'turn on  %m.lights', 'send_on', "all lights"],
            ['w', 'turn off  %m.lights', 'send_off', "all lights"],
            ['w', 'toggle  %m.lights', 'send_toggle', "all lights"],
            ['w', 'set  color of %m.lights to %m.colors', 'set_color', "all lights", "red"],
            ['w', 'fade %m.lights to %m.colors', 'fade_color', "all lights", "red"],
            ['w', 'fade in %m.lights', 'fade_in', "all lights"],
            ['w', 'fade out %m.lights ', 'fade_out', "all lights"],
            ['w', 'set  fade speed to %m.speeds', 'set_fade_speed', 'slow'],
            ['w', 'set  brightness of %m.lights to %m.power', 'set_power', "all lights", 'high'],
            // Motor cmds
            ['w', 'turn on motor', 'send_motor_on'],
            ['w', 'turn off motor', 'send_motor_off'],
            ['w', 'reverse motor direction', 'send_motor_rev'],
            ['w', 'toggle motor', 'send_motor_toggle'],
            ['w', 'set motor speed %m.speeds', 'set_speed', "slow"],
            // Sensor cmds
            ['h', 'when %m.sensors', 'poll_sensor', "sensor clips are connected"]
            //            ['R', 'Value of %m.sensors', 'poll', "Sensor A"]
        ],

        url: 'https://github.com/bsb20/scratch-to-serial/tree/gh-pages',
        menus: {
            power: ['low', 'medium', 'high'],
            colors: ['red', 'orange', 'yellow', 'green', 'blue', 'purple', 'white', 'surprise'],
            lights: ["all lights", "light 1", "light 2", "light 3"],
            speeds: ["slow", "faster", "fastest"],
            sensors: ["sensor clips are connected", "sensor clips are disconnected", "light shines on sensor", "shadow falls on sensor"]
        }
    };

    // Register the extension..
    ScratchExtensions.register('Sample extension', descriptor, ext);
})({});
