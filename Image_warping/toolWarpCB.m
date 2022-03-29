function toolWarpCB(varargin)

%   Copyright © 2021, Renjie Chen @ USTC

hlines = evalin('base', 'hToolPoint.UserData');
im = evalin('base', 'im');
limg = evalin('base', 'limg');
rimg = evalin('base', 'rimg');
glimg = evalin('base', 'glimg');

p2p = zeros(numel(hlines)*2,2); 
for i=1:numel(hlines)
    p2p(i*2+(-1:0),:) = hlines(i).getPosition();
end

[im2,paint] = RBFImageWarp(im, p2p(1:2:end,:), p2p(2:2:end,:));
im3 = RRBFImageWarp(im, p2p(1:2:end,:), p2p(2:2:end,:));
im4 = Interpolation(im2,paint);
set(limg, 'CData', im2);
set(rimg, 'CData', im3);
set(glimg, 'CData', im4);