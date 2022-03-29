function [im2,paint] = RBFNImageWarp(im, psrc, pdst)


% input: im, psrc, pdst


%% basic image manipulations
% get image (matrix) size
[h, w, dim] = size(im);
[n, ~]=size(psrc);
im2 = zeros(h,w,dim,'uint8');
paint=zeros(h,w);
if n==0
    return
end
psrc(:,[1,2])=psrc(:,[2,1]);
pdst(:,[1,2])=pdst(:,[2,1]);
d=sum(sum((pdst-psrc).^2));


%% TODO: compute warpped image
D=zeros(n,n);
for i=1:n
    for j=1:n
        D(i,j)=1./(sum((psrc(i,:)-psrc(j,:)).^2)+d);
    end
end
B=D\(pdst-psrc);

key1=transpose(meshgrid(1:h,1:w));
key2=meshgrid(1:w,1:h);
p1=zeros(h,w);
p2=zeros(h,w);
for k=1:n
    q=1./((key1-psrc(k,1)).^2+(key2-psrc(k,2)).^2+d);
    p1=p1+B(k,1)*q;
    p2=p2+B(k,2)*q;
end
p1=round(p1+key1);
p2=round(p2+key2);

for i=1:h
    for j=1:w
        if p1(i,j)>0 & p1(i,j)<h+1 & p2(i,j)>0 & p2(i,j)<1+w
            im2(p1(i,j),p2(i,j),:)=im(i,j,:);
            paint(p1(i,j),p2(i,j))=1;
        end
    end
end
end

