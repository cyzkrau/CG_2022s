function output = uniform_func(f, v)

np = size(v, 1);
[B, H] = findBoundary(v, f); nB = size(B, 2);

next = sparse([f(:,1);f(:,1);f(:,2)],[f(:,2);f(:,3);f(:,3)],1,np,np); 
next = next+transpose(next); next(next~=0) = 1; 

sol = -transpose(next./sum(next)); sol(B,:) = 0; sol = sol+sparse(1:np,1:np,1,np,np);
dst = sin(sparse(B,2*ones(1,nB),1:nB,np,2)*2*pi/nB) + cos(sparse(B,ones(1,nB),1:nB,np,2)*2*pi/nB);
dst = dst.*(sparse(B,ones(1,nB),1,np,2)+sparse(B,2*ones(1,nB),1,np,2));

output = sol\full(dst);
end

