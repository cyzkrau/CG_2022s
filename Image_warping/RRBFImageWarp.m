function im3 = RRBFNImageWarp(im, psrc, pdst)


% input: im, psrc, pdst


%% basic image manipulations
% get image (matrix) size
[h, w, dim] = size(im);
[n, ~]=size(psrc);
im3 = zeros(h,w,dim,'uint8')+255;
if n==0
    return
end
psrc(:,[1,2])=psrc(:,[2,1]);
pdst(:,[1,2])=pdst(:,[2,1]);
d=0;
for i=1:n
    d=d+sum((pdst(i,:)-psrc(i,:)).^2);
end


%% TODO: compute warpped image
for i=1:n
    for j=1:n
        D(i,j)=1./(sum((pdst(i,:)-pdst(j,:)).^2)+d);
    end
end
B=D\(psrc-pdst);

key1=transpose(meshgrid(1:h,1:w));
key2=meshgrid(1:w,1:h);
p1=zeros(h,w);
p2=zeros(h,w);
for k=1:n
    q=1./((key1-pdst(k,1)).^2+(key2-pdst(k,2)).^2+d);
    p1=p1+B(k,1)*q;
    p2=p2+B(k,2)*q;
end
p1=round(p1+key1);
p2=round(p2+key2);

for i=1:h
    for j=1:w
        if p1(i,j)<1 | p1(i,j)>h | p2(i,j)<1 | p2(i,j)>w
            im3(i,j,:)=[0,0,0];
        else
            im3(i,j,:)=im(p1(i,j),p2(i,j),:);
    end
end
end

