function [h, f] = bezier(h, f)
%BEZIER 此处显示有关此函数的摘要
%   此处显示详细说明
    t = 0:0.001:1;
    if nargin < 2
        f = figure('Name', 'Bezier');
        xlim([-5 5]);
        ylim([-5 5]);
    else
        figure(f);
    end
    if nargin == 0
        h = drawpolyline;
    end
    n = size(h.Position,1);
    c = zeros(1,n);
    for i=1:n
        c(i)=nchoosek(n-1, i-1);
    end
    pmat = ((t'.^(n-1:-1:0)).*((1-t)'.^(0:1:n-1))).*c;
    hold on;
    hcurve = plot(pmat*(h.Position*[1;1i]), 'g', 'linewidth', 2);
    h.addlistener('MovingROI', @(h, evt)findans(evt.CurrentPosition, hcurve, pmat));

    function p = findans(p,h,mt)
        p=mt*(p*[1;1i]);
        set(h,'xdata',real(p),'ydata',imag(p));
    end
end

