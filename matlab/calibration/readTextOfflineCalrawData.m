% readTextMagCalData.m
%
% read raw magnetometer calibration data recorded in a text file by hedrot
% 
% Copyright 2017 Alexis Baskind

function offlineCalrawData = readTextOfflineCalrawData(filename)

  fileID = fopen(filename,'r');

  [VAL, COUNT, ERRMSG] = fscanf (fileID, "%i, %i %i %i;\n");
  M=reshape(VAL,4,COUNT/4)';
  
  rawMagCalData = M(:,2:4);
  
  fclose(fileID);

endfunction