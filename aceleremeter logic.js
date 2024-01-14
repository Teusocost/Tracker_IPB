// Acelerometro logic

var type = null;

var X = null, 
    Y = null,
    Z = null;

const value1 = -10,
      value2 = -5,
      value3 = 0,
      value4 = 5,
      value5 = 10;  

//analise:
if(X >= valuen && Y >= valuen && Z >= valuen)       type = 1;
else if(X >= valuen && Y >= valuen && Z >= valuen)  type = 2;

msg.payload = [
    {
        measurement: "Accelerometer",
        fields: {
            Position: type
        },
        tags: {
            sensorID: "all",
            location: "local_test"
        }
    }
];

return msg;

console.log();