close all;
clear all;

[audio fs] = audioread('testaudio.wav');
x = (0:length(audio)-1)/fs;

%c = 0:96;
%coeficientes = -7.9184e-8*(c*6000/96).^2 + 2.4031e-5*(c*6000/96) + 0.017;
%figure
%plot(c/96 * 6000,coeficientes);


LARGO_FRAME = 400; % 25 ms
RET_FRAME = 160;
for i=0:(length(audio)-LARGO_FRAME)/RET_FRAME
    frame = audio((1:LARGO_FRAME)+i*RET_FRAME);

    frame_spec = frame;% abs(fft(frame));
    %frame_spec(1:4) = 0; %<250 Hz
    %frame_spec(96:256) = 0; %>6000 Hz
    
    frame_spec_norm = frame_spec/sum(frame_spec);
    %histogram(frame_spec_norm,50);
    %plot(frame_spec_norm)
    
    
    histogram(frame_spec,50);

    pause(eps);
    
    entr = 0;
    for j=1:length(frame_spec_norm)
        if frame_spec_norm(j) > 0 
            entr = entr - frame_spec_norm(j)*log(frame_spec_norm(j));
        end
    end
    entropia(i+1) = entr;
    
end

%Forma anterior de calculo de energia
%audio_2 = audio.^2;
%energia_filtrada = filter(ones(fs*0.1,1),[1], audio_2);

figure
hold all;
%plotyy(x,audio,(1:length(entropia))*LARGO_FRAME/fs,entropia);
subplot(2,1,1);
plot(x,audio);
subplot(2,1,2);
plot((1:length(entropia))*RET_FRAME/fs,entropia);