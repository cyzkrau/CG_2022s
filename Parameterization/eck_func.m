function output = eck_func(f, v)

np = size(v, 1); ne = size(f, 1);
[B, H] = findBoundary(v, f); nB = size(B, 2);

l(:,1)=vecnorm(v(f(:,2),:)-v(f(:,3),:),2,2);
l(:,2)=vecnorm(v(f(:,1),:)-v(f(:,3),:),2,2);
l(:,3)=vecnorm(v(f(:,1),:)-v(f(:,2),:),2,2);

c(:,1)=(l(:,2).^2+l(:,3).^2-l(:,1).^2)./(2*l(:,2).*l(:,3));
c(:,2)=(l(:,1).^2+l(:,3).^2-l(:,2).^2)./(2*l(:,1).*l(:,3));
c(:,3)=(l(:,1).^2+l(:,2).^2-l(:,3).^2)./(2*l(:,1).*l(:,2));
c = c./sqrt(1-c.^2); 

next = sparse([f(:,1);f(:,1)],[f(:,2);f(:,3)],[c(:,3);c(:,2)],np,np); 
next = next+sparse([f(:,2);f(:,2)],[f(:,1);f(:,3)],[c(:,3);c(:,1)],np,np); 
next = next+sparse([f(:,3);f(:,3)],[f(:,1);f(:,2)],[c(:,2);c(:,1)],np,np); 

sol = -next./sum(next,2); sol(B,:) = 0; sol = sol+sparse(1:np,1:np,1,np,np);
dst = sin(sparse(B,2*ones(1,nB),1:nB,np,2)*2*pi/nB) + cos(sparse(B,ones(1,nB),1:nB,np,2)*2*pi/nB);
dst = dst.*(sparse(B,ones(1,nB),1,np,2)+sparse(B,2*ones(1,nB),1,np,2));

output = sol\full(dst);
end

