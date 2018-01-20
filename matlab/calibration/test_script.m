maxError = .05; # maximum radius deviation


rawMagCalData = readTextOfflineCalrawData("/Users/baskind/Desktop/headtrackerRawMagCalibrationData.txt");

# estimation 1 (free ellipsoid)
[ center1, radii1, evecs1, v1 ] = ellipsoid_fit( rawMagCalData, 0 )

# calculate and show calibrated data
MagCalData1 = (rawMagCalData - ones(size(rawMagCalData,1),1)*center1')./(ones(size(rawMagCalData,1),1)*radii1')*evecs1;
figure;plot3(MagCalData1(:,1),MagCalData1(:,2),MagCalData1(:,3),'o');
xlim([-1.5 1.5]);ylim([-1.5 1.5]);zlim([-1.5 1.5]);
title("points after calibration, WITH rotation");

# calculate and show radius error
radius_error1 = sqrt(MagCalData1(:,1).^2+MagCalData1(:,2).^2+MagCalData1(:,3).^2);
figure;plot(radius_error1);
title("error after calibration, WITH rotation");ylim([.8 1.2]);grid
filteredIndexes = find(abs(radius_error1-1)<=maxError);
min(radius_error1(filteredIndexes)), max(radius_error1(filteredIndexes))


# estimation 2 (non rotated ellipsoid)
[ center2, radii2, evecs2, v2 ] = ellipsoid_fit( rawMagCalData, 1 )

# calculate and show calibrated data
MagCalData2 = (rawMagCalData - ones(size(rawMagCalData,1),1)*center2')./(ones(size(rawMagCalData,1),1)*radii2');
figure;plot3(MagCalData2(:,1),MagCalData2(:,2),MagCalData2(:,3),'o');
xlim([-1.5 1.5]);ylim([-1.5 1.5]);zlim([-1.5 1.5]);
title("points after calibration, NO rotation");

# calculate and show radius error
radius_error2 = sqrt(MagCalData2(:,1).^2+MagCalData2(:,2).^2+MagCalData2(:,3).^2);
figure;plot(radius_error2);
title("error after calibration, NO rotation");ylim([.8 1.2]);grid
filteredIndexes = find(abs(radius_error2-1)<=maxError);
min(radius_error2(filteredIndexes)), max(radius_error2(filteredIndexes))

# estimation 3 (non rotated ellipsoid, second pass after removing the farther points)
rawMagCalDataFiltered = rawMagCalData(filteredIndexes,:);
[ center3, radii3, evecs3, v3 ] = ellipsoid_fit( rawMagCalDataFiltered, 1 )

# calculate and show calibrated data
MagCalData3 = (rawMagCalDataFiltered - ones(size(rawMagCalDataFiltered,1),1)*center3')./(ones(size(rawMagCalDataFiltered,1),1)*radii3');
figure;plot3(MagCalData3(:,1),MagCalData3(:,2),MagCalData3(:,3),'o');
xlim([-1.5 1.5]);ylim([-1.5 1.5]);zlim([-1.5 1.5]);
title("points after calibration, NO rotation, second pass");

# calculate and show radius error
radius_error3 = sqrt(MagCalData3(:,1).^2+MagCalData3(:,2).^2+MagCalData3(:,3).^2);
figure;plot(radius_error3);
title("error after calibration, NO rotation, second pass");ylim([.8 1.2]);grid
min(radius_error3), max(radius_error3)
