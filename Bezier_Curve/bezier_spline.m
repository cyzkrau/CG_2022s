function [h, f] = bezier_spline(h, f)
%BEZIER_SPLINE 此处显示有关此函数的摘要
%   此处显示详细说明
    if nargin < 2
        f = figure('Name', 'Bezier Spline');
        xlim([-5 5]);
        ylim([-5 5]);
        hold on;
    else
        figure(f);
    end
    if nargin == 0
        h = drawpolyline;
        hcurve = plot(findans(h.Position), 'g', 'linewidth', 2);
        h.addlistener('MovingROI', @(h, evt)findans(evt.CurrentPosition, hcurve));
    else 
        pts = scatter(h.Position(:,1),h.Position(:,2),40,'MarkerEdgeColor',[0 0 .5],...
              'MarkerFaceColor',[0 0 .7],'LineWidth',1.5);
        hold on;
        hcurve = plot(findans(h.Position), 'g', 'linewidth', 2);
        h.addlistener('MovingROI', @(h, evt)findans(evt.CurrentPosition, hcurve, pts));
    end

    function p = findans(p,h,s)
        if nargin == 3
            set(s,'xdata',p(:,1),'ydata',p(:,2));
        end
        p=p*[1;1i];
        p=spline(1:length(p),p,1:0.01:length(p));
        if nargin>1
            set(h,'xdata',real(p),'ydata',imag(p));
        end
    end
end

