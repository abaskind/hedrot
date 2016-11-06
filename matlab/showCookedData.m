% showCookedData.m
% 
% Copyright 2016 Alexis Baskind

function showCookedData(headtrackerData)
  figure;

  subplot(4,1,1);
  plot(headtrackerData.data.gyroCalData);
  title("Gyroscope calibrated data");
  
  subplot(4,1,2);
  plot(headtrackerData.data.magCalData);
  title("Magnetometer calibrated data");
  
  subplot(4,1,3);
  plot(headtrackerData.data.accCalData);
  title("Accelerometer calibrated data");
  
  subplot(4,1,4);
  plot(headtrackerData.data.yawPitchRoll);
  title("Estimated Angles");legend("yaw","pitch","roll");
  
endfunction
