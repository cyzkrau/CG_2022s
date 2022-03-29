%   Copyright © 2021, Renjie Chen @ USTC


%% read image
im = imread('MonaLisa.jpg');
% im(:,:,1) = uint8(kron(round(randn(10,10)*255),ones(50,50)));
% im(:,:,2) = uint8(kron(round(randn(10,10)*255),ones(50,50)));
% im(:,:,3) = uint8(kron(round(randn(10,10)*255),ones(50,50)));

%% draw 2 copies of the image
figure('Units', 'pixel', 'Position', [100,100,1000,700], 'toolbar', 'none');
subplot(221); imshow(im); title({'Source image', 'Press the red tool button to add point-point constraints'});
subplot(222); limg = imshow(im*0); title({'Warpped Image By Original RBF', 'Press the blue tool button to compute the warpped image'});
subplot(223); rimg = imshow(im*0); title({'Warpped Image By Reverse RBF', 'Press the blue tool button to compute the warpped image'});
subplot(224); glimg = imshow(im*0); title({'After Interpolation', 'Press the blue tool button to compute the warpped image'});
hToolPoint = uipushtool('CData', reshape(repmat([1 0 0], 100, 1), [10 10 3]), 'TooltipString', 'add point constraints to the map', ...
                        'ClickedCallback', @toolPositionCB, 'UserData', []);
hToolWarp = uipushtool('CData', reshape(repmat([0 0 1], 100, 1), [10 10 3]), 'TooltipString', 'compute warped image', ...
                       'ClickedCallback', @toolWarpCB);
