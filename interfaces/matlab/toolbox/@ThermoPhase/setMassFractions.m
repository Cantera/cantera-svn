function setMassFractions(tp, y, norm)
%
%
%

if isa(y,'double')
    if nargin == 3
        if strcmp(norm,'nonorm')
            phase_set(tp.tp_id,23,y);
        else
            phase_set(tp.tp_id,21,y);
        end
    else
        phase_set(tp.tp_id,21,y);
    end
    %
    % string input
    %
elseif isa(y,'char')
    phase_set(tp.tp_id,31,y);
end
