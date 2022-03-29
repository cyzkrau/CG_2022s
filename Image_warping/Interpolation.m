function im4 = Interpolation(im,paint)
[h, w, dim] = size(im);
im4=double(im);
for i=2:h-1
    for j=2:w-1
        if paint(i,j)==0
            if any(paint(i-1,j-1:j+1)==1) | any(paint(i+1,j-1:j+1)==1)
                im4(i,j,:)=sum(sum(im4(i-1:i+1,j-1:j+1,:).*paint(i-1:i+1,j-1:j+1)))...
                    /sum(sum(paint(i-1:i+1,j-1:j+1)));
            elseif any(paint(i-1:i+1,j-1)==1) | any(paint(i-1:i+1,j+1)==1)
                im4(i,j,:)=sum(sum(im4(i-1:i+1,j-1:j+1,:).*paint(i-1:i+1,j-1:j+1)))...
                    /sum(sum(paint(i-1:i+1,j-1:j+1)));
            else
                im4(i,j,:)=[0,0,0];
            end
        end
    end
end
im4=uint8(im4);
end