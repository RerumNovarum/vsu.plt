settings.outformat = "pdf";
settings.render=16;
import three;
size(10cm);
defaultpen(fontsize(10pt));
currentprojection = perspective(5*(2,3,8),up=Y);

draw(O -- 3X, arrow=Arrow3(), L=Label("$X_w$", position=EndPoint));
draw(O -- 2Y, arrow=Arrow3(), L=Label("$Y_w$", position=EndPoint));
draw(O -- 4Z, arrow=Arrow3(), L=Label("$Z_w$", position=EndPoint));
dot(O, L=Label("$0$", align=S));

triple OC = 1X + 2Z + 1Y;
triple OS = 4X + -3Z + 4.5Y;
triple T = -.5X + Z + Y;
triple N = (OC-OS)/2;
real D = abs(OC-OS);

triple kk = N/length(N);
triple jj = (T - dot(kk,T)*kk); jj = jj/length(jj);
triple ii = cross(jj, kk);

real screenW = 6;
real screenH = 6;
triple screenLB = OS - .5screenW*ii - .5screenH*jj;
triple screenLT=screenLB+screenH*jj;
triple screenRB=screenLB+screenW*ii;
triple screenRT=screenRB+screenH*jj;

path3 screen = plane(O=screenLB, screenW*ii, screenH*jj);
draw(screen, gray);
draw(OC -- screenLB, dashed+gray);
draw(OC -- screenRB, dashed+gray);
draw(OC -- screenLT, dashed+gray);
draw(OC -- screenRT, dashed+gray);

pen longLabelsPen = fontsize(6pt);
draw(shift(OC) * scale(0.4, 0.4, 0.4) * unitsphere);
label(Label("$\mathtt{C}$",longLabelsPen), OC-.6Y);
dot(OS, L=Label("$\mathtt{S}$",longLabelsPen,align=S));
draw(OC -- OS);
draw(OS -- (OS+N),  arrow=Arrow3(HookHead3), L=Label("$N$", position=EndPoint),darkblue);
draw(OC -- (OC+T),  arrow=Arrow3(HookHead3), L=Label("$T$", position=EndPoint),darkblue);
draw(OS -- (OS+ii), arrow=Arrow3(HookHead3), L=Label("$X_v$", position=EndPoint));
draw(OS -- (OS+jj), arrow=Arrow3(HookHead3), L=Label("$Y_v$", position=EndPoint));
draw(OS -- (OS+kk), arrow=Arrow3(HookHead3), L=Label("$Z_v$", position=EndPoint));

triple pt = OC - 1.7kk + .5ii - .7jj;
triple ptproj = pt - OS;
triple ptView = (dot(ii,ptproj), dot(jj,ptproj), dot(kk,ptproj));
triple ptprojorth = OS + ii*ptView.x + jj*ptView.y;
triple ptProjView = D/(D-ptView.z)*ptView; ptProjView = (ptProjView.x, ptProjView.y, 0);
ptproj = OS+ii*ptProjView.x+jj*ptProjView.y;

draw(ptproj -- ptprojorth, black);
draw(OC -- ptproj, arrow=Arrow3(HookHead3),darkbrown);
draw(pt -- ptprojorth, arrow=Arrow3(HookHead3),darkbrown);
draw(ptproj -- (OS + ptProjView.x*ii) -- OS, dashed+gray);
draw(ptproj -- (OS + ptProjView.y*jj) -- OS, dashed+gray);
draw(ptprojorth -- (OS + ptView.x*ii) -- OS, dashed+darkgray);
draw(ptprojorth -- (OS + ptView.y*jj) -- OS, dashed+darkgray);
dot(pt, L=Label("$P$",p=longLabelsPen,align=S));
dot(ptprojorth, L=Label("$P_\mathtt{orthogonal}$",p=longLabelsPen,align=E));
dot(ptproj, L=Label("$P_\mathtt{perspective}$",p=longLabelsPen,align=E));
