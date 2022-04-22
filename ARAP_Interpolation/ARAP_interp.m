function xout = ARAP_interp(w, dH, A, theta, ddt)
    nt = size(theta, 1); rhs = zeros(2*nt, 2);
    rA = (1-w)*reshape(kron(ones(1,nt),eye(2)),2,2,nt)+w*A;
    c = cos(w*theta); s = sin(w*theta);
    
    for i=1:nt
        rA(:,:,i) = [c(i),-s(i);s(i),c(i)]*rA(:,:,i);
        rhs(2*i-1:2*i,:) = rA(:,:,i)*ddt(:,:,i);
    end
    newrhs = [rhs(1:2:2*nt,:);rhs(2:2:2*nt,:)];    
    xout = dH\newrhs;