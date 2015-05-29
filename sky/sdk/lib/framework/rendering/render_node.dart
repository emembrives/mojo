// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import '../node.dart';
import 'dart:sky' as sky;

class ParentData {
  void detach() {
    detachSiblings();
  }
  void detachSiblings() { } // workaround for lack of inter-class mixins in Dart
  void merge(ParentData other) {
    // override this in subclasses to merge in data from other into this
    assert(other.runtimeType == this.runtimeType);
  }
}

const kLayoutDirections = 4;

double clamp({double min: 0.0, double value: 0.0, double max: double.INFINITY}) {
  assert(min != null);
  assert(value != null);
  assert(max != null);

  if (value > max)
    value = max;
  if (value < min)
    value = min;
  return value;
}

class RenderNodeDisplayList extends sky.PictureRecorder {
  RenderNodeDisplayList(double width, double height) : super(width, height);
  void paintChild(RenderNode child, sky.Point position) {
    save();
    translate(position.x, position.y);
    child.paint(this);
    restore();
  }
}

abstract class RenderNode extends AbstractNode {

  // LAYOUT

  // parentData is only for use by the RenderNode that actually lays this
  // node out, and any other nodes who happen to know exactly what
  // kind of node that is.
  ParentData parentData;
  void setParentData(RenderNode child) {
    // override this to setup .parentData correctly for your class
    if (child.parentData is! ParentData)
      child.parentData = new ParentData();
  }

  void adoptChild(RenderNode child) { // only for use by subclasses
    // call this whenever you decide a node is a child
    assert(child != null);
    setParentData(child);
    super.adoptChild(child);
  }
  void dropChild(RenderNode child) { // only for use by subclasses
    assert(child != null);
    assert(child.parentData != null);
    child.parentData.detach();
    super.dropChild(child);
  }

  static List<RenderNode> _nodesNeedingLayout = new List<RenderNode>();
  static bool _debugDoingLayout = false;
  bool _needsLayout = true;
  bool get needsLayout => _needsLayout;
  RenderNode _relayoutSubtreeRoot;
  dynamic _constraints;
  dynamic get constraints => _constraints;
  bool debugAncestorsAlreadyMarkedNeedsLayout() {
    if (_relayoutSubtreeRoot == null)
      return true; // we haven't yet done layout even once, so there's nothing for us to do
    RenderNode node = this;
    while (node != _relayoutSubtreeRoot) {
      assert(node._relayoutSubtreeRoot == _relayoutSubtreeRoot);
      assert(node.parent != null);
      node = node.parent as RenderNode;
      if (!node._needsLayout)
        return false;
    }
    assert(node._relayoutSubtreeRoot == node);
    return true;
  }
  void markNeedsLayout() {
    assert(!_debugDoingLayout);
    assert(!debugDoingPaint);
    if (_needsLayout) {
      assert(debugAncestorsAlreadyMarkedNeedsLayout());
      return;
    }
    _needsLayout = true;
    assert(_relayoutSubtreeRoot != null);
    if (_relayoutSubtreeRoot != this) {
      assert(parent is RenderNode);
      parent.markNeedsLayout();
    } else {
      _nodesNeedingLayout.add(this);
    }
  }
  static void flushLayout() {
    _debugDoingLayout = true;
    List<RenderNode> dirtyNodes = _nodesNeedingLayout;
    _nodesNeedingLayout = new List<RenderNode>();
    dirtyNodes..sort((a, b) => a.depth - b.depth)..forEach((node) {
      if (node._needsLayout && node.attached)
        node._doLayout();
    });
    _debugDoingLayout = false;
  }
  void _doLayout() {
    try {
      assert(_relayoutSubtreeRoot == this);
      performLayout();
    } catch (e, stack) {
      print('Exception raised during layout of ${this}: ${e}');
      print(stack);
      return;
    }
    _needsLayout = false;
  }
  void layout(dynamic constraints, { bool parentUsesSize: false }) {
    RenderNode relayoutSubtreeRoot;
    if (!parentUsesSize || sizedByParent || parent is! RenderNode)
      relayoutSubtreeRoot = this;
    else
      relayoutSubtreeRoot = parent._relayoutSubtreeRoot;
    if (!needsLayout && constraints == _constraints && relayoutSubtreeRoot == _relayoutSubtreeRoot)
      return;
    _constraints = constraints;
    _relayoutSubtreeRoot = relayoutSubtreeRoot;
    if (sizedByParent)
      performResize();
    performLayout();
    _needsLayout = false;
    markNeedsPaint();
  }
  bool get sizedByParent => false; // return true if the constraints are the only input to the sizing algorithm (in particular, child nodes have no impact)
  void performResize(); // set the local dimensions, using only the constraints (only called if sizedByParent is true)
  void performLayout();
    // Override this to perform relayout without your parent's
    // involvement.
    //
    // This is called during layout. If sizedByParent is true, then
    // performLayout() should not change your dimensions, only do that
    // in performResize(). If sizedByParent is false, then set both
    // your dimensions and do your children's layout here.
    //
    // When calling layout() on your children, pass in
    // "parentUsesSize: true" if your size or layout is dependent on
    // your child's size.

  // when the parent has rotated (e.g. when the screen has been turned
  // 90 degrees), immediately prior to layout() being called for the
  // new dimensions, rotate() is called with the old and new angles.
  // The next time paint() is called, the coordinate space will have
  // been rotated N quarter-turns clockwise, where:
  //    N = newAngle-oldAngle
  // ...but the rendering is expected to remain the same, pixel for
  // pixel, on the output device. Then, the layout() method or
  // equivalent will be invoked.

  void rotate({
    int oldAngle, // 0..3
    int newAngle, // 0..3
    Duration time
  }) { }


  // PAINTING

  static bool debugDoingPaint = false;
  void markNeedsPaint() {
    assert(!debugDoingPaint);
    // TODO(abarth): It's very redundant to call this for every node in the
    // render tree during layout. We should instead compute a summary bit and
    // call it once at the end of layout.
    sky.view.scheduleFrame();
  }
  void paint(RenderNodeDisplayList canvas) { }


  // HIT TESTING

  void handlePointer(sky.PointerEvent event) {
    // override this if you have a client, to hand it to the client
    // override this if you want to do anything with the pointer event
  }

  // RenderNode subclasses are expected to have a method like the
  // following (with the signature being whatever passes for coordinates
  // for this particular class):
  // bool hitTest(HitTestResult result, { sky.Point position }) {
  //   // If (x,y) is not inside this node, then return false. (You
  //   // can assume that the given coordinate is inside your
  //   // dimensions. You only need to check this if you're an
  //   // irregular shape, e.g. if you have a hole.)
  //   // Otherwise:
  //   // For each child that intersects x,y, in z-order starting from the top,
  //   // call hitTest() for that child, passing it /result/, and the coordinates
  //   // converted to the child's coordinate origin, and stop at the first child
  //   // that returns true.
  //   // Then, add yourself to /result/, and return true.
  // }
  // You must not add yourself to /result/ if you return false.

}

class HitTestResult {
  final List<RenderNode> path = new List<RenderNode>();

  RenderNode get result => path.first;

  void add(RenderNode node) {
    path.add(node);
  }
}


// GENERIC MIXIN FOR RENDER NODES WITH ONE CHILD

abstract class RenderNodeWithChildMixin<ChildType extends RenderNode> {
  ChildType _child;
  ChildType get child => _child;
  void set child (ChildType value) {
    if (_child != null)
      dropChild(_child);
    _child = value;
    if (_child != null)
      adoptChild(_child);
    markNeedsLayout();
  }
}


// GENERIC MIXIN FOR RENDER NODES WITH A LIST OF CHILDREN

abstract class ContainerParentDataMixin<ChildType extends RenderNode> {
  ChildType previousSibling;
  ChildType nextSibling;
  void detachSiblings() {
    if (previousSibling != null) {
      assert(previousSibling.parentData is ContainerParentDataMixin<ChildType>);
      assert(previousSibling != this);
      assert(previousSibling.parentData.nextSibling == this);
      previousSibling.parentData.nextSibling = nextSibling;
    }
    if (nextSibling != null) {
      assert(nextSibling.parentData is ContainerParentDataMixin<ChildType>);
      assert(nextSibling != this);
      assert(nextSibling.parentData.previousSibling == this);
      nextSibling.parentData.previousSibling = previousSibling;
    }
    previousSibling = null;
    nextSibling = null;
  }
}

abstract class ContainerRenderNodeMixin<ChildType extends RenderNode, ParentDataType extends ContainerParentDataMixin<ChildType>> implements RenderNode {
  // abstract class that has only InlineNode children

  bool _debugUltimatePreviousSiblingOf(ChildType child, { ChildType equals }) {
    assert(child.parentData is ParentDataType);
    while (child.parentData.previousSibling != null) {
      assert(child.parentData.previousSibling != child);
      child = child.parentData.previousSibling;
      assert(child.parentData is ParentDataType);
    }
    return child == equals;
  }
  bool _debugUltimateNextSiblingOf(ChildType child, { ChildType equals }) {
    assert(child.parentData is ParentDataType);
    while (child.parentData.nextSibling != null) {
      assert(child.parentData.nextSibling != child);
      child = child.parentData.nextSibling;
      assert(child.parentData is ParentDataType);
    }
    return child == equals;
  }

  ChildType _firstChild;
  ChildType _lastChild;
  void add(ChildType child, { ChildType before }) {
    assert(child != this);
    assert(before != this);
    assert(child != before);
    assert(child != _firstChild);
    assert(child != _lastChild);
    adoptChild(child);
    assert(child.parentData is ParentDataType);
    assert(child.parentData.nextSibling == null);
    assert(child.parentData.previousSibling == null);
    if (before == null) {
      // append at the end (_lastChild)
      child.parentData.previousSibling = _lastChild;
      if (_lastChild != null) {
        assert(_lastChild.parentData is ParentDataType);
        _lastChild.parentData.nextSibling = child;
      }
      _lastChild = child;
      if (_firstChild == null)
        _firstChild = child;
    } else {
      assert(_firstChild != null);
      assert(_lastChild != null);
      assert(_debugUltimatePreviousSiblingOf(before, equals: _firstChild));
      assert(_debugUltimateNextSiblingOf(before, equals: _lastChild));
      assert(before.parentData is ParentDataType);
      if (before.parentData.previousSibling == null) {
        // insert at the start (_firstChild); we'll end up with two or more children
        assert(before == _firstChild);
        child.parentData.nextSibling = before;
        before.parentData.previousSibling = child;
        _firstChild = child;
      } else {
        // insert in the middle; we'll end up with three or more children
        // set up links from child to siblings
        child.parentData.previousSibling = before.parentData.previousSibling;
        child.parentData.nextSibling = before;
        // set up links from siblings to child
        assert(child.parentData.previousSibling.parentData is ParentDataType);
        assert(child.parentData.nextSibling.parentData is ParentDataType);
        child.parentData.previousSibling.parentData.nextSibling = child;
        child.parentData.nextSibling.parentData.previousSibling = child;
        assert(before.parentData.previousSibling == child);
      }
    }
    markNeedsLayout();
  }
  void remove(ChildType child) {
    assert(child.parentData is ParentDataType);
    assert(_debugUltimatePreviousSiblingOf(child, equals: _firstChild));
    assert(_debugUltimateNextSiblingOf(child, equals: _lastChild));
    if (child.parentData.previousSibling == null) {
      assert(_firstChild == child);
      _firstChild = child.parentData.nextSibling;
    } else {
      assert(child.parentData.previousSibling.parentData is ParentDataType);
      child.parentData.previousSibling.parentData.nextSibling = child.parentData.nextSibling;
    }
    if (child.parentData.nextSibling == null) {
      assert(_lastChild == child);
      _lastChild = child.parentData.previousSibling;
    } else {
      assert(child.parentData.nextSibling.parentData is ParentDataType);
      child.parentData.nextSibling.parentData.previousSibling = child.parentData.previousSibling;
    }
    child.parentData.previousSibling = null;
    child.parentData.nextSibling = null;
    dropChild(child);
    markNeedsLayout();
  }
  void redepthChildren() {
    ChildType child = _firstChild;
    while (child != null) {
      redepthChild(child);
      assert(child.parentData is ParentDataType);
      child = child.parentData.nextSibling;
    }
  }
  void attachChildren() {
    ChildType child = _firstChild;
    while (child != null) {
      child.attach();
      assert(child.parentData is ParentDataType);
      child = child.parentData.nextSibling;
    }
  }
  void detachChildren() {
    ChildType child = _firstChild;
    while (child != null) {
      child.detach();
      assert(child.parentData is ParentDataType);
      child = child.parentData.nextSibling;
    }
  }

  ChildType get firstChild => _firstChild;
  ChildType get lastChild => _lastChild;
  ChildType childAfter(ChildType child) {
    assert(child.parentData is ParentDataType);
    return child.parentData.nextSibling;
  }
}