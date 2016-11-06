% readTextDataFromHeadtracker.m
%
% read raw and cooked data recorded in a text file by hedrot
% 
% Copyright 2016 Alexis Baskind

function headtrackerData = readTextDataFromHeadtracker(filename)

  fileID = fopen(filename,'r');

  % 1° read the header
  readHeaderFlag = 1;
  while(readHeaderFlag && ((txt = fgetl (fileID)) != -1))
   if(strcmp(txt,"</header>"))
      % did we reach the end of the header?
      readHeaderFlag = 0;
   elseif(strcmp(txt,"<header>"))
      % is it the beginning of the header? If yes, just ignore the line
   else
      a = strsplit (txt, {', ', ';'});
      key = a{1};
      value = a{2};
      %split the key between spaces
      numberOfItems = length(strsplit(value));
      
      %case 1: single value
      if(numberOfItems == 1)
        eval(sprintf("headtrackerData.header.%s = %s;",key,value));
      else
        %case 2: several values: save as array
        eval(sprintf("headtrackerData.header.%s = [%s];",key,value));
      endif
    endif
  endwhile
    
    
  %2° read the data
  [VAL, COUNT, ERRMSG] = fscanf (fileID, "%i, %i %i %i %i %i %i %i %i %i %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f;\n");
  M=reshape(VAL,29,COUNT/29)';
  
  headtrackerData.data.magRawData = M(:,2:4);
  headtrackerData.data.accRawData = M(:,5:7);
  headtrackerData.data.gyroRawData = M(:,8:10);
  headtrackerData.data.magCalData = M(:,11:13);
  headtrackerData.data.accCalData = M(:,14:16);
  headtrackerData.data.accCalDataLP = M(:,17:19);
  headtrackerData.data.gyroCalData = M(:,20:22);
  headtrackerData.data.quaternion = M(:,23:26);
  headtrackerData.data.yawPitchRoll = M(:,27:29);
  

  headtrackerData.numberOfSamples = size(M,1);
  
  fclose(fileID);

endfunction