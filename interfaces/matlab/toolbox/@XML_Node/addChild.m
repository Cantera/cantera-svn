function n = addChild(root, id, val)
% Add a child to the root.
%
% :param root:
%     Instance of class :mat:func:`XML_Node`
% :param id:
%     String ID of the child to be added.
% :param val:
%     String value to be added to the child.
% :return:
%     Instance of class :mat:func:`XML_Node`
%

n = ctmethods(10, 11, root.id, id, val);
