warning off;
[x,t] = readObj('elephant_s');
y = readObj('elephant_t');
% x=[0 0 0;2 0 0;-1 1 0;-1 -1 0];
% y=[0 0 0;-1 1 0;2 0 0;-1 -1 0];
% t=[1 2 3;1 2 4;1 3 4];
nt = size(t, 1);

%% set figure
amd = figure('position', [10 40 1210, 840]); subplot(131); drawmesh(t, x);
subplot(133); drawmesh(t, y);
subplot(132); h=drawmesh(t, x);

%% pre work out matrix
x=x(:,1:2);y=y(:,1:2);
A=zeros(2,2,nt); ddt=zeros(2,2,nt);
theta=zeros(nt,1);
for i=1:nt
    q = ([x(t(i,1),:)-x(t(i,2),:); x(t(i,1),:)-x(t(i,3),:)]...
        \ [y(t(i,1),:)-y(t(i,2),:); y(t(i,1),:)-y(t(i,3),:)])';
    [s,v,d] = svd(q);
    A(:,:,i) = d'*v*d; s=s*d;
    theta(i) = atan2(s(1,2), s(1,1));
    ddt(:,:,i) = [x(t(i,1),:)-x(t(i,2),:);x(t(i,1),:)-x(t(i,3),:)];
end

%% set angle in right num
B = [1];
conmap = sparse([t(:,1);t(:,1);t(:,2);t(:,2);t(:,3);t(:,3)],...
    [t(:,2);t(:,3);t(:,1);t(:,3);t(:,1);t(:,2)],ones(6*nt,1)) ~= 0;
flag = zeros(nt, 1);
flag(B) = 1;
while B
    fj = find(sum(t==t(B(1),1),2)+sum(t==t(B(1),2),2)+sum(t==t(B(1),3),2)==2)';
    fj(flag(fj)==1)=[];
    for i=fj
        while theta(B(1)) - theta(i) > pi
            theta(i) = theta(i) + 2*pi;
        end
        while theta(B(1)) - theta(i) < -pi
            theta(i) = theta(i) - 2*pi;
        end
    end
    flag(fj) = 1;
    B = [B, fj]; B(1) = [];
end

%% pre decomposition matrix
weight = ones(size(theta));
% weight = exp(-(theta-mean(theta)).^2);
ddt = ddt.*reshape(weight,1,1,nt);

jIndex = [t(:,1),t(:,2),t(:,1),t(:,3)];
iIndex = [1:nt,1:nt,nt+(1:nt),nt+(1:nt)];
vValue = [weight;-weight;weight;-weight];
H = sparse(iIndex,jIndex,vValue);
dH = decomposition(H);

%% show result
nframe = 100;
% filename = 'result.gif';
for w = [0:1/nframe:1,1:-1/nframe:1/nframe]
% for w = [0:1/nframe:1]
    z = ARAP_interp(w, dH, A, theta, ddt);
    set(h,'vertices',z);
    drawnow; %pause(0.01);
    
%     frame = getframe(amd);
%     im = frame2im(frame); 
%     [imind,cm] = rgb2ind(im,256);
%     if w==0
%         imwrite(imind,cm,filename,'gif','WriteMode','overwrite', 'Loopcount',inf);
%     else
%         imwrite(imind,cm,filename,'gif','WriteMode','append','DelayTime',0.02);
%     end
end

%% function of show
function h = drawmesh(t, x)
    h = trimesh(t, x(:,1), x(:,2), x(:,1), 'facecolor', 'interp', 'edgecolor', 'k');
    axis equal; axis off; view(2);
end
