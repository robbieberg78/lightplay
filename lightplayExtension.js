(function(ext) {

    var PLUGIN_URL = 'http://localhost:8000';
    //Device UUID and Characteristics
    var UUID = "6e400001b5a3f393e0a9e50e24dcca9e";
    var RX = "6e400003b5a3f393e0a9e50e24dcca9e";
    var TX = "6e400002b5a3f393e0a9e50e24dcca9e";



    var CHANNELS = ["all lights", "light 1", "light 2", "light 3", "motor", "Sensor A", "Sensor B"];
    var SENSORS = ["Light", "Clip"];
    var LEVELS = ["Low", "Med", "High"];
    var PROTOCOL = {
        LIGHT: 0,
        MOTOR: 1,
        FADE_TO: 2,
        OTHER: 3,
        SET: 4,
    
        COLOR: {
            RED: 0,
            ORANGE: 1,
            YELLOW: 2,
            GREEN: 3,
            BLUE: 4,
            PURPLE: 5,
            WHITE: 6,
            SURPRISE: 7
        },
    
        SPEED: {
            SLOW: 0,
            FASTER: 1,
            FASTEST: 2
        },
    
        LIGHTCOMMAND: {  
            ON: 0,
            OFF: 1,
            FADE_IN: 2,
            FADE_OUT: 3,
            TOGGLE: 4,
            POWER_LOW: 5,
            POWER_MED: 6,
            POWER_HIGH: 7
        },
    
        MOTORCOMMAND: {
            ON: 0,
            OFF: 1,
            REVERSE: 2,
            TOGGLE: 3,
            SET_LOW: 4,
            SET_MED: 5,
            SET_HIGH: 6
        },

        OTHERCOMMAND: {
            FADE: 0,
            RGB: 1
        },
    
        CHANNEL: {
            ALL: 0,
            ONE: 1,
            TWO: 2,
            THREE: 3,
            MOTOR: 4
        }
    };

    var MOTOR_SPEEDS = {
        slow: PROTOCOL.MOTORCOMMAND.SET_LOW,
        faster: PROTOCOL.MOTORCOMMAND.SET_MED,
        fastest: PROTOCOL.MOTORCOMMAND.SET_HIGH
    };

    var FADE_TIMEOUTS = {
        slow: 10100, 
        faster: 5100,
        fastest: 1100
    };

    var FADE_SPEEDS = {
        slow: PROTOCOL.SPEED.SLOW, 
        faster: PROTOCOL.SPEED.FASTER,
        fastest: PROTOCOL.SPEED.FASTEST
    };


    var BRIGHTNESS = {
        low: PROTOCOL.LIGHTCOMMAND.POWER_LOW,
        medium: PROTOCOL.LIGHTCOMMAND.POWER_MED,
        high: PROTOCOL.LIGHTCOMMAND.POWER_HIGH
    };
    
    var COLORS = {
        red: PROTOCOL.COLOR.RED,
        orange: PROTOCOL.COLOR.ORANGE,
        yellow: PROTOCOL.COLOR.YELLOW,
        green: PROTOCOL.COLOR.GREEN,
        blue: PROTOCOL.COLOR.BLUE,
        purple: PROTOCOL.COLOR.PURPLE,
        white: PROTOCOL.COLOR.WHITE,
        surprise: PROTOCOL.COLOR.SURPRISE
    };

    var device = null;


    ext.fade_speed = 1100;

    ext._getStatus = function () {

        if (device) {
            if (device.is_open()) {
                return {status: 2, msg: 'Lightplay connected'};
            }
            else {
                return {status: 1, msg: 'Lightplay connecting...'};
            }
        }
        else {
            return {status: 1, msg: 'Lightplay disconnected'};
        }
    };

    ext._deviceConnected = function (dev) {
        if (device) return;
        console.log(dev.ext_name + " gotten by extension");
        device = dev;
        device.open(function (d) {
            if (device == d) {
                device.set_sensor_handler(null);
            }
            else if (d) {
                console.log('Received open callback for wrong device');
            }
            else {
                console.log('Opening device failed');
                device = null;
            }
        });
    };

     ext._deviceRemoved = function (dev) {
        if (device != dev) return;
        device = null;
    };


    function compile (type, channel, action){
        return (type << 5) | (channel << 3) | action;
    }

    function send_to_channel(type, channel, action, callback) {
        if (CHANNELS.indexOf(channel) >= 0) {
            channel = CHANNELS.indexOf(channel);
        }
        device.emit('write', {uuid: TX, bytes: [compile(type, channel, action)]});
        if(callback){
            callback();
        }
    }

    ext.send_on = function(channel, callback) {
        console.log("turn on");
        send_to_channel(PROTOCOL.LIGHT, channel, PROTOCOL.LIGHTCOMMAND.ON, callback);

    };

    ext.send_off = function(channel, callback) {
        send_to_channel(PROTOCOL.LIGHT, channel, PROTOCOL.LIGHTCOMMAND.OFF, callback);
    };

    ext.send_toggle = function(channel, callback) {
        send_to_channel(PROTOCOL.LIGHT, channel, PROTOCOL.LIGHTCOMMAND.TOGGLE, callback);
    };

    ext.send_motor_on = function(callback) {
        send_to_channel(PROTOCOL.MOTOR, 0, PROTOCOL.MOTORCOMMAND.ON, callback);
    };

    ext.send_motor_off = function(callback) {
        send_to_channel(PROTOCOL.MOTOR, 0, PROTOCOL.MOTORCOMMAND.OFF, callback);
    };

    ext.send_motor_toggle = function(callback) {
        send_to_channel(PROTOCOL.MOTOR, 0, PROTOCOL.MOTORCOMMAND.TOGGLE, callback);
    };

    ext.set_speed = function(level, callback) {
        send_to_channel(PROTOCOL.MOTOR, 0, MOTOR_SPEEDS[level], callback);
    };

    ext.send_motor_rev = function(callback) {
        send_to_channel(PROTOCOL.MOTOR, 0, PROTOCOL.MOTORCOMMAND.REVERSE, callback);
    };

    ext.poll_sensor = function(sensor, callback) {
        if (descriptor.menus.sensors.indexOf(sensor) >= 0) {
            var sensorID = descriptor.menus.sensors.indexOf(sensor) / 2;
            var action = descriptor.menus.sensors.indexOf(sensor) % 2;
            device.once(RX, function(data){
                callback({
                predicate: "predicate",
                });
            });
        }
    };

    ext.predicate = function(sensor, evt){
        return true;
    };

    ext.set_power = function(channel, level, callback) {
        send_to_channel(PROTOCOL.LIGHT, channel, BRIGHTNESS[level], callback);
    };

    ext.set_fade_speed = function(speed, callback) {
        ext.fade_speed = FADE_TIMEOUTS[speed];
        send_to_channel(PROTOCOL.OTHER, FADE_SPEEDS[speed], PROTOCOL.OTHERCOMMAND.FADE, callback);
    };

    ext.set_rgb_color = function(channel, red, green, blue, callback) {
        send_to_channel(PROTOCOL.OTHER, channel, PROTOCOL.OTHERCOMMAND.RGB);
        device.emit('write', {uuid: TX, bytes: [red, green, blue]});
        callback()
    };

    ext.fade_in = function(channel, callback) {
        send_to_channel(PROTOCOL.LIGHT, channel, PROTOCOL.LIGHTCOMMAND.FADE_IN, function(data) {
            setTimeout(callback, ext.fade_speed, data);
        });

    };

    ext.fade_out = function(channel, callback) {
        send_to_channel(PROTOCOL.LIGHT, channel, PROTOCOL.LIGHTCOMMAND.FADE_OUT, function(data) {
            setTimeout(callback, ext.fade_speed, data);
        });
    };

    ext.set_color = function(channel, color, callback) {
        send_to_channel(PROTOCOL.SET, channel, COLORS[color], callback);
    };

    ext.fade_color = function(channel, color, callback) {
        send_to_channel(PROTOCOL.FADE_TO, channel, COLORS[color], function(data) {
            setTimeout(callback, ext.fade_speed, data);
        });
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
            ['w', 'set  %m.lights color to %n %n %n', 'set_rgb_color', "all lights", '100', '100', '100'],
            ['w', 'set  brightness of %m.lights to %m.power', 'set_power', "all lights", 'high'],
            // Motor cmds
            ['w', 'turn on motor', 'send_motor_on'],
            ['w', 'turn off motor', 'send_motor_off'],
            ['w', 'reverse motor direction', 'send_motor_rev'],
            ['w', 'toggle motor', 'send_motor_toggle'],
            ['w', 'set motor speed %m.speeds', 'set_speed', "slow"],
            // Sensor cmds
            ['H', 'when %m.sensors', 'poll_sensor', "sensor clips are connected"],
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
    var device_info = {uuid: [UUID]};
    var rx = {}
    rx[RX] = {notify: true},
    device_info["read_characteristics"] = rx;
    var tx = {};
    tx[TX] = {};
    device_info["write_characteristics"] = tx;
    ScratchExtensions.register('Lightplay 1', descriptor, ext, { info: device_info, type: 'ble' });

})({});
