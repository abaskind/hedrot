% showHeadtrackerDataSpectrum.m
%
% Copyright 2016 Alexis Baskind

function showHeadtrackerDataSpectrum (headtrackerData, stream, Nsamples, windowtype)
  if(nargin<3)
    Nsamples = headtrackerData.numberOfSamples;
  endif
  
  if(nargin<4)
    windowtype = "hanning";
  endif
  
  if(strcmp(windowtype,"rect"))
    window = ones(Nsamples,3);
  else
    eval(strcat("window = ",windowtype,"(Nsamples)*ones(1,3);"));
  endif
  
  %Nfft=pow2(nextpow2(headtrackerData.numberOfSamples));
  Nfft = 65536;
  %Nfft=headtrackerData.numberOfSamples;
  f=(0:Nfft-1)/Nfft*headtrackerData.header.samplerate;
  eval(strcat("X=fft(headtrackerData.data.",stream,"(1:Nsamples,:).*window,Nfft,1);"));
  
  figure;plot(f(1:Nfft/2+1),20*log10(abs(X(1:Nfft/2+1,:))));grid
  xlabel("frequency (Hz)");ylabel("amplitude (dB)");
endfunction
