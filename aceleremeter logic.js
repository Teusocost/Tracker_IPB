// Acelerometro logic

var type = null;

var X = null, 
    Y = null,
    Z = null;

//analise:
//front
if(X >= -3   && X < 3   &&
   Y >= -2.5 && Y < 4.5 &&
   Z >=  7   && Z < 11            
)type = 1;
else
if (X >= 4   && X < 12    &&
    Y >= -2  && Y < 4     &&
    Z >=  -4   && Z < 2            
)type = 2;
else
if (X >= 9   && X < 12    &&
    Y >=  -3   && Y < 3   &&
    Z >=  -3   && Z < 1            
)type = 3;

//left
else
if (X >= -1   && X < 2    &&
    Y >=  3   && Y < 8   &&
    Z >=  4   && Z < 9            
)type = 4;
else
if (X >= -1   && X < 2    &&
    Y >=  8   && Y < 12   &&
    Z >=  -6   && Z < 5            
)type = 5;
//behind
if (X >= -3   && X < -7    &&
    Y >=  -3  && Y < 3   &&
    Z >=  4  && Z < 8            
)type = 6;
else
if (X >= -2   && X < 2    &&
    Y >=  8  && Y < 12   &&
    Z >=  -4  && Z < 1            
)type = 7;
//Riht
if (X >= -1   && X < 3    &&
    Y >=  -6  && Y < 1   &&
    Z >=  6  && Z < 12            
)type = 8;
else
if (X >= -2   && X < 2    &&
    Y >=  -12  && Y < -6   &&
    Z >=  0  && Z < 6            
)type = 9;
//else if(X >= valuen && Y >= valuen && Z >= valuen)   type = 2;
/*
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
*/