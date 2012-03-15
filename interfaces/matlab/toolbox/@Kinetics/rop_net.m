function q = rop_net(a)
% ROP_F  Forward rates of progress for all reactions.
%
%    Q = ROP_F(K)
%
%        Returns a column vector of the forward rates of progress
%        for all reactions.
%
%    See also: rop_r, rop_net.
%
q = kinetics_get(a.id,13,0);
if nargout == 0
    figure
    set(gcf,'Name','Rates of Progress')
    bar(q)
    xlabel('Reaction Number')
    ylabel('Net Rate of Progress [kmol/m^3]')
    title('Net Rates of Progress')
end
