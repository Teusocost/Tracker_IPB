// Acelerometro logic

var type = null;

var X = -1.06, 
    Y = -9.85,
    Z = -1.1;

//analise:
//top
if (X >= -3 && X < 3 &&
    Y >= -2.5 && Y < 4.5 &&
    Z >= 7 && Z < 11
) type = 1;
//front
else
    if (X >= 4 && X < 12 &&
        Y >= -2 && Y < 6 &&
        Z >= -4 && Z < 7
    ) type = 2;
    else
        if (X >= 9 && X < 12 &&
            Y >= -3 && Y < 3 &&
            Z >= -3 && Z < 1
        ) type = 3;

        //left
        else
            if (X >= -1 && X < 2 &&
                Y >= 3 && Y < 8 &&
                Z >= 4 && Z < 9
            ) type = 4;
            else
                if (X >= -1 && X < 2 &&
                    Y >= 8 && Y < 12 &&
                    Z >= -6 && Z < 5
                ) type = 5;
                else
                    //behind
                    if (X >= -9 && X < -3 &&
                        Y >= -3 && Y < 3 &&
                        Z >= 4 && Z < 7
                    ) type = 6;
                    else
                        if (X >= -12 && X < 6 &&
                            Y >= -1 && Y < 3 &&
                            Z >= -2 && Z < 2
                        ) type = 7;
                        else
                            //Riht
                            if (X >= -1 && X < 3 &&
                                Y >= -6 && Y < 1 &&
                                Z >= 6 && Z < 12
                            ) type = 8;
                            else
                                if (X >= -2 && X < 2 &&
                                    Y >= -12 && Y < -6 &&
                                    Z >= 0 && Z < 6
                                ) type = 9;
console.log(type);
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