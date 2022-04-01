[v, f, vt, vn] = readObj('cathead.obj', false); np = size(v, 1);

uni = uniform_func(f, v);
flo = floater_func(f, v);
eck = eck_func(f,v);
drawmesh(f, v);
set(gcf,'name','3d');
drawmesh(f, uni);
set(gcf,'name','tutte');
drawmesh(f, flo);
set(gcf,'name','floater');
drawmesh(f, eck);
set(gcf,'name','eck');
