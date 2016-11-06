% computeAnglesMadgwick.m
%
% octave/matlab version of angle estimation in libhedrot
%
% Part of code is derived from Sebastian Madgwick's open-source gradient descent angle estimation algorithm
% 
% Copyright 2016 Alexis Baskind

function newHeadtrackerData = computeAnglesMadgwick(headtrackerData)
  newHeadtrackerData = headtrackerData;
  
  SamplePeriod = 1/newHeadtrackerData.header.samplerate;
  alphaMin = newHeadtrackerData.header.accLPalphaMin;
  alphaGain = newHeadtrackerData.header.accLPalphaGain;
  BetaMax = newHeadtrackerData.header.MadgwickBetaMax;
  BetaGain = newHeadtrackerData.header.MadgwickBetaGain;
  
  accLPstate = newHeadtrackerData.data.accCalData(1, :);
  
  %the first value is left as it was calculated by the headtracker receiver (initialization)
  for t = 2:length(newHeadtrackerData.data.gyroCalData(:,1))
      [newHeadtrackerData.data.quaternion(t, :), newHeadtrackerData.data.step(t, :), accLPstate] = Update(newHeadtrackerData.data.quaternion(t-1, :), SamplePeriod, alphaMin, alphaGain, BetaMax, BetaGain, accLPstate, newHeadtrackerData.data.gyroCalData(t,:), newHeadtrackerData.data.accCalData(t,:), newHeadtrackerData.data.magCalData(t,:));
  end

  eulerAngles = quatern2euler(quaternConj(newHeadtrackerData.data.quaternion)) * (180/pi); %roll pitch yaw
  newHeadtrackerData.data.yawPitchRoll = [eulerAngles(:,3) eulerAngles(:,2) eulerAngles(:,1)];
endfunction


%% method (derived from MadgwickAHRS.m without classdef)
function [outputQuaternion, step, accLPstate] = Update(inputQuaternion, SamplePeriod, alphaMin, alphaGain, BetaMax, BetaGain, oldAccLPstate, GyroscopeData, AccelerometerData, MagnetometerData)
    q = inputQuaternion; % short name local variable for readability

    % compute gyro_norm2
    gyro_norm2 = GyroscopeData(1)^2 + GyroscopeData(2)^2 + GyroscopeData(3)^2;
    
    % lowpass Accelerometer data
    accLPalpha = alphaMin + (1 - alphaMin) * min(max(alphaGain * gyro_norm2,0),1);
    AccelerometerDataLP(1) = accLPalpha * AccelerometerData(1) + (1 - accLPalpha) * oldAccLPstate(1);
    AccelerometerDataLP(2) = accLPalpha * AccelerometerData(2) + (1 - accLPalpha) * oldAccLPstate(2);
    AccelerometerDataLP(3) = accLPalpha * AccelerometerData(3) + (1 - accLPalpha) * oldAccLPstate(3);
    %filter state update
    accLPstate(1) = AccelerometerDataLP(1);
    accLPstate(2) = AccelerometerDataLP(2);
    accLPstate(3) = AccelerometerDataLP(3);
    
    % Normalise Accelerometer measurement
    if(norm(AccelerometerData) == 0), return; end	% handle NaN
    AccelerometerDataLP = AccelerometerDataLP / norm(AccelerometerDataLP);	% normalise magnitude

    % Normalise Magnetometer measurement
    if(norm(MagnetometerData) == 0), return; end	% handle NaN
    MagnetometerData = MagnetometerData / norm(MagnetometerData);	% normalise magnitude

    % Reference direction of Earth's magnetic feild
    h = quaternProd(q, quaternProd([0 MagnetometerData], quaternConj(q)));
    b = [0 norm([h(2) h(3)]) 0 h(4)];

    % Gradient decent algorithm corrective step
    F = [2*(q(2)*q(4) - q(1)*q(3)) - AccelerometerDataLP(1)
        2*(q(1)*q(2) + q(3)*q(4)) - AccelerometerDataLP(2)
        2*(0.5 - q(2)^2 - q(3)^2) - AccelerometerDataLP(3)
        2*b(2)*(0.5 - q(3)^2 - q(4)^2) + 2*b(4)*(q(2)*q(4) - q(1)*q(3)) - MagnetometerData(1)
        2*b(2)*(q(2)*q(3) - q(1)*q(4)) + 2*b(4)*(q(1)*q(2) + q(3)*q(4)) - MagnetometerData(2)
        2*b(2)*(q(1)*q(3) + q(2)*q(4)) + 2*b(4)*(0.5 - q(2)^2 - q(3)^2) - MagnetometerData(3)];
    J = [-2*q(3),                 	2*q(4),                    -2*q(1),                         2*q(2)
        2*q(2),                 	2*q(1),                    	2*q(4),                         2*q(3)
        0,                         -4*q(2),                    -4*q(3),                         0
        -2*b(4)*q(3),               2*b(4)*q(4),               -4*b(2)*q(3)-2*b(4)*q(1),       -4*b(2)*q(4)+2*b(4)*q(2)
        -2*b(2)*q(4)+2*b(4)*q(2),	2*b(2)*q(3)+2*b(4)*q(1),	2*b(2)*q(2)+2*b(4)*q(4),       -2*b(2)*q(1)+2*b(4)*q(3)
        2*b(2)*q(3),                2*b(2)*q(4)-4*b(4)*q(2),	2*b(2)*q(1)-4*b(4)*q(3),        2*b(2)*q(2)];
    step = (J'*F);
    step = step / norm(step);	% normalise step magnitude

    
    % compute beta 
    Beta = BetaMax * (1 - min(max(BetaGain * gyro_norm2,0),1));
    
    % Compute rate of change of quaternion
    qDot = 0.5 * quaternProd(q, [0 GyroscopeData(1) GyroscopeData(2) GyroscopeData(3)]) - Beta * step';

    % Integrate to yield quaternion
    q = q + qDot * SamplePeriod;
    outputQuaternion = q / norm(q); % normalise quaternion
end
