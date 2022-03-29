function imret = blendImagePoisson(im1, im2, roi, targetPosition)

% input: im1 (background), im2 (foreground), roi (in im2), targetPosition (in im1)

%% TODO: compute blended image
k = 0; % how much backround
[h1, w1, c] = size(im1);
[h2, w2, c] = size(im2);

persistent h3; persistent w3; persistent nmask;
persistent l; persistent u; persistent d;
q1 = size(roi)-[1,0];
if ~isequal(d, roi(2:q1(1)+1,:)-roi(1:q1(1),:)) % roi shape changed
    d = roi(2:q1(1)+1,:)-roi(1:q1(1),:);
    
    w3 = ceil(max(roi(:,1)-min(roi(:,1))))+1;
    h3 = ceil(max(roi(:,2)-min(roi(:,2))))+1;
    mask = inpolygon(repmat(1:h3, 1,w3), kron(1:w3,ones(1,h3)), roi(:,2)-min(roi(:,2)), roi(:,1)-min(roi(:,1)));

    masking = zeros(1, h3*w3);
    masking(mask) = 1;
    masking = reshape(masking, h3, w3);
    masking = [zeros(h3,1),masking(:,1:w3-1)]+[zeros(1,w3);masking(1:h3-1,:)]...
        +masking+[masking(:,2:w3),zeros(h3,1)]+[zeros(1,w3);masking(2:h3,:)];
    masking(masking ~= 5) = 0; masking = -masking/20;
    masking = reshape(masking, h3*w3, 1);
    nmask = [1+4*masking, 1+4*masking, 1+4*masking];
    mat = spdiags([[masking(h3+1:h3*w3);zeros(h3,1)], [masking(2:h3*w3);zeros(1,1)], ones(h3*w3, 1),...
        [zeros(1,1);masking(1:h3*w3-1)], [zeros(h3,1);masking(1:h3*w3-h3)]], [-h3, -1, 0, 1, h3], h3*w3, h3*w3);
    [l, u] = lu(mat);
end

left1 = floor(min(targetPosition(:,1))); up1 = floor(min(targetPosition(:,2)));
left2 = floor(min(roi(:,1))); up2 = floor(min(roi(:,2)));

dsts = reshape(double(im1(up1:up1+h3-1,left1:left1+w3-1,:)), h3*w3, c);
srcs = reshape(double(im2(up2:up2+h3-1,left2:left2+w3-1,:)), h3*w3, c);
oridst = dsts;
dsts = dsts - srcs - k*oridst;
dsts = dsts .* nmask;
dsts = u\(l\dsts);
dsts = dsts + srcs + k*oridst;
% dsts = dsts .* nmask;
dsts = uint8(reshape(dsts, h3, w3, c));
imret = im1;
imret(up1:up1+h3-1,left1:left1+w3-1,:) = dsts;
