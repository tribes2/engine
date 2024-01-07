//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "Editor/creator.h"
#include "dgl/dgl.h"

IMPLEMENT_CONOBJECT(CreatorTree);

//------------------------------------------------------------------------------
// Class CreatorTree::Node
//------------------------------------------------------------------------------

CreatorTree::Node::Node() :
   mFlags(0),
   mParent(0),
   mName(0),
   mValue(0),
   mId(0),
   mTab(0)
{
   VECTOR_SET_ASSOCIATION(mChildren);
}

CreatorTree::Node::~Node()
{
   for(U32 i = 0; i < mChildren.size(); i++)
      delete mChildren[i];
}

//------------------------------------------------------------------------------

void CreatorTree::Node::expand(bool exp)
{
   if(exp)
   {
      if(mParent)
         mParent->expand(exp);
      mFlags.set(Node::Expanded);
   }
   else if(!isRoot())
   {  
      if(isGroup())
         for(U32 i = 0; i < mChildren.size(); i++)
            mChildren[i]->expand(exp);
      
      mFlags.clear(Selected);
      mFlags.clear(Expanded);
   }
}

//------------------------------------------------------------------------------

CreatorTree::Node * CreatorTree::Node::find(S32 id)
{
   if(mId == id)
      return(this);
   
   if(!isGroup())
      return(0);
      
   for(U32 i = 0; i < mChildren.size(); i++)
   {
      Node * node = mChildren[i]->find(id);
      if(node)
         return(node);
   }

   return(0);
}

//------------------------------------------------------------------------------

bool CreatorTree::Node::isFirst()
{
   AssertFatal(!isRoot(), "CreatorTree::Node::isFirst - cannot call on root node");
   return(this == mParent->mChildren[0]);
}

bool CreatorTree::Node::isLast()
{
   AssertFatal(!isRoot(), "CreatorTree::Node::isLast - cannot call on root node");
   return(this == mParent->mChildren[mParent->mChildren.size()-1]);
}

bool CreatorTree::Node::hasChildItem()
{
   for(U32 i = 0; i < mChildren.size(); i++)
   {  
      if(mChildren[i]->isGroup() && mChildren[i]->hasChildItem())
         return(true);

      if(!mChildren[i]->isGroup())
         return(true);
   }
   
   return(false);
}

S32 CreatorTree::Node::getSelected()
{
   for(U32 i = 0; i < mChildren.size(); i++)
   {
      if(mChildren[i]->isSelected())
         return(mChildren[i]->mId);
      else if(mChildren[i]->isGroup())
      {
         S32 ret = mChildren[i]->getSelected();
         if(ret != -1)
            return(ret);
      }
   }
   return(-1);
}

//------------------------------------------------------------------------------
// Class CreatorTree
//------------------------------------------------------------------------------
CreatorTree::CreatorTree() :
   mCurId(0), 
   mTxtOffset(5), 
   mRoot(0)
{  
   VECTOR_SET_ASSOCIATION(mNodeList);
   clear();            
}

CreatorTree::~CreatorTree()
{
   delete mRoot;
}

//------------------------------------------------------------------------------

CreatorTree::Node * CreatorTree::createNode(const char * name, const char * value, bool group, Node * parent)
{
   Node * node = new Node();
   node->mId = mCurId++;
   node->mName = name ? StringTable->insert(name) : 0;
   node->mValue = value ? StringTable->insert(value) : 0;
   node->mFlags.set(Node::Group, group);
   
   // add to the parent group
   if(parent)
   {
      node->mParent = parent;
      if(!addNode(parent, node))
      {
         delete node;
         return(0);
      }
   }
   
   return(node);
}

//------------------------------------------------------------------------------

void CreatorTree::clear()
{  
   delete mRoot;
   mCurId = 0;
   mRoot = createNode(0, 0, true);
   mRoot->mFlags.set(Node::Root | Node::Expanded);
   mSize = Point2I(1,0);
}

//------------------------------------------------------------------------------

bool CreatorTree::addNode(Node * parent, Node * node)
{
   if(!parent->isGroup())
      return(false);
      
   //
   parent->mChildren.push_back(node);
   return(true);
}

//------------------------------------------------------------------------------

CreatorTree::Node * CreatorTree::findNode(S32 id)
{
   return(mRoot->find(id));
}

//------------------------------------------------------------------------------

void CreatorTree::sort()
{
   // groups then items by alpha
}

//------------------------------------------------------------------------------

static S32 cAddGroup(SimObject * obj, S32, const char ** argv)
{
   CreatorTree * tree = dynamic_cast<CreatorTree*>(obj);
   CreatorTree::Node * grp = tree->findNode(dAtoi(argv[2]));
   
   if(!grp || !grp->isGroup())
      return(-1);

   // return same named group if found...
   for(U32 i = 0; i < grp->mChildren.size(); i++)
      if(!dStricmp(argv[3], grp->mChildren[i]->mName))
         return(grp->mChildren[i]->mId);
      
   CreatorTree::Node * node = tree->createNode(argv[3], 0, true, grp);
   tree->build();
   return(node ? node->getId() : -1);
}

static S32 cAddItem(SimObject * obj, S32, const char ** argv)
{
   CreatorTree * tree = dynamic_cast<CreatorTree*>(obj);
   CreatorTree::Node * grp = tree->findNode(dAtoi(argv[2]));
   
   if(!grp || !grp->isGroup())
      return -1;

   CreatorTree::Node * node = tree->createNode(argv[3], argv[4], false, grp);
   tree->build();
   return(node ? node->getId() : -1);
}

//------------------------------------------------------------------------------

static bool cFileNameMatch(SimObject *, S32, const char ** argv)
{
   // argv[2] - world short
   // argv[3] - type short
   // argv[4] - filename

   // interior filenames
   // 0     - world short ('b', 'x', ...)
   // 1->   - type short ('towr', 'bunk', ...)
   U32 typeLen = dStrlen(argv[3]);
   if(dStrlen(argv[4]) < (typeLen + 1))
      return(false);
   
   // world
   if(dToupper(argv[4][0]) != dToupper(argv[2][0]))
      return(false);
      
   return(!dStrnicmp(argv[4]+1, argv[3], typeLen));
}

static S32 cGetSelected(SimObject * obj, S32, const char **)
{
   CreatorTree * tree = dynamic_cast<CreatorTree*>(obj);
   return(tree->getSelected());
}

static bool cIsGroup(SimObject * obj, S32, const char ** argv)
{
   CreatorTree * tree = dynamic_cast<CreatorTree*>(obj);
   CreatorTree::Node * node = tree->findNode(dAtoi(argv[2]));
   if(node && node->isGroup())
      return(true);
   return(false);
}

static const char * cGetName(SimObject * obj, S32, const char ** argv)
{
   CreatorTree * tree = dynamic_cast<CreatorTree*>(obj);
   CreatorTree::Node * node = tree->findNode(dAtoi(argv[2]));
   return(node ? node->mName : 0);
}

static const char * cGetValue(SimObject * obj, S32, const char ** argv)
{
   CreatorTree * tree = dynamic_cast<CreatorTree*>(obj);
   CreatorTree::Node * node = tree->findNode(dAtoi(argv[2]));
   return(node ? node->mValue : 0);
}

static void cClear(SimObject * obj, S32, const char **)
{
   CreatorTree * tree = dynamic_cast<CreatorTree*>(obj);
   tree->clear();
}

static S32 cGetParent(SimObject * obj, S32, const char ** argv)
{
   CreatorTree * tree = dynamic_cast<CreatorTree*>(obj);
   CreatorTree::Node * node = tree->findNode(dAtoi(argv[2]));
   if(node && node->mParent)
      return(node->mParent->getId());
   else
      return(-1);
}
//------------------------------------------------------------------------------

void CreatorTree::consoleInit()
{
   Con::addCommand("CreatorTree", "fileNameMatch", cFileNameMatch, "creator.fileNameMatch(world, type, filename);", 5, 5);
   Con::addCommand("CreatorTree", "addGroup", cAddGroup, "creator.addGroup(parent, group);", 4, 4);
   Con::addCommand("CreatorTree", "addItem", cAddItem, "creator.addItem(group, name, value);", 5, 5);
   Con::addCommand("CreatorTree", "getSelected", cGetSelected, "creator.getSelected();", 2, 2);
   Con::addCommand("CreatorTree", "isGroup", cIsGroup, "creator.isGroup(id);", 3, 3);
   Con::addCommand("CreatorTree", "getName", cGetName, "creator.getName(id);", 3, 3);
   Con::addCommand("CreatorTree", "getValue", cGetValue, "creator.getValue(id);", 3, 3);
   Con::addCommand("CreatorTree", "clear", cClear, "creator.clear();", 2, 2);
   Con::addCommand("CreatorTree", "getParent", cGetParent, "creator.getParent(id);", 3, 3);
}

//------------------------------------------------------------------------------

void CreatorTree::buildNode(Node * node, U32 tab)
{
   if(node->isExpanded())
      for(U32 i = 0; i < node->mChildren.size(); i++)
      {
         Node * child = node->mChildren[i];
         child->mTab = tab;
         child->select(false);
         mNodeList.push_back(child);
            
         // grab width
         if(bool(mProfile->mFont) && child->mName)
         {
            S32 width = (tab + 1) * mTabSize + mProfile->mFont->getStrWidth(child->mName) + mTxtOffset; 
            if(width > mMaxWidth)
               mMaxWidth = width;
         }
         
         if(node->mChildren[i]->isGroup())
            buildNode(node->mChildren[i], tab+1);
      }
}

//------------------------------------------------------------------------------

void CreatorTree::build()
{
   mMaxWidth = 0;
   mNodeList.clear();
   buildNode(mRoot, 0);
   mCellSize.set(mMaxWidth+1, mBitmapBounds[BmpParentContinue].extent.y);
   setSize(Point2I(1, mNodeList.size()));
}

//------------------------------------------------------------------------------
bool CreatorTree::onWake()
{
   if(!Parent::onWake())
      return(false);
      
   if(!mProfile->mTextureHandle.getBitmap())
      return(false);
      
   if(!createBitmapArray(mProfile->mTextureHandle.getBitmap(), mBitmapBounds, 1, BmpCount))
      return(false);
      
   mTabSize = mBitmapBounds[BmpParentContinue].extent.x;

   //
   build();
   mCellSize.set(mMaxWidth + 1, mBitmapBounds[BmpParentContinue].extent.y);
   setSize(Point2I(1, mNodeList.size()));
   return true;
}

//------------------------------------------------------------------------------

void CreatorTree::onMouseDown(const GuiEvent & event)
{
   if(!mActive)
   {
      Parent::onMouseDown(event);
      return;
   }
 
   Point2I pos = globalToLocalCoord(event.mousePoint);
   
   bool dblClick = event.mouseClickCount > 1;
   
   // determine cell
   Point2I cell(pos.x < 0 ? -1 : pos.x / mCellSize.x, pos.y < 0 ? -1 : pos.y / mCellSize.y);
   if(cell.x >= 0 && cell.x < mSize.x && cell.y >= 0 && cell.y < mSize.y)
   {
      Node * node = mNodeList[cell.y];
      S32 offset = mTabSize * node->mTab;
      if(node->isGroup() && node->mChildren.size() && pos.x >= offset && pos.x <= (offset + mTabSize))
      {
         node->expand(!node->isExpanded());
         build();
         dblClick = false;
      }
      
      if(pos.x >= offset)
      {
         if(dblClick)
            node->expand(!node->isExpanded());
         build();
         node->select(true);
      }
   }
}

//------------------------------------------------------------------------------

void CreatorTree::onMouseDragged(const GuiEvent & event)
{
   event;
}

//------------------------------------------------------------------------------

void CreatorTree::onRenderCell(Point2I offset, Point2I cell, bool, bool)
{
   Node * node =  mNodeList[cell.y];

   // get the bitmap index...
   S32 bmpIndex = BmpNone;
   if(node->isGroup())
   {
      if(node->isExpanded() || !node->mChildren.size())
         bmpIndex = BmpParentOpen;
      else bmpIndex = BmpParentClosed;
   }

   //   
   if(!node->isFirst())
      bmpIndex += 2;
   if(!node->isLast())
      bmpIndex += 1;
   
   //
   if(bmpIndex != BmpNone)
   {
      Point2I pos(offset.x + mTabSize * (mNodeList[cell.y]->mTab), offset.y);
      dglClearBitmapModulation();
      dglDrawBitmapSR(mProfile->mTextureHandle, pos, mBitmapBounds[bmpIndex]);
   }

   // draw the vertical line thingies
   Node * parent = node->mParent;
   while(!parent->isRoot())
   {
      if(!parent->isLast())
      {
         Point2I pos(offset.x + mTabSize * parent->mTab, offset.y);
         dglClearBitmapModulation();
         dglDrawBitmapSR(mProfile->mTextureHandle, pos, mBitmapBounds[BmpParentContinue]);
      }
      parent = parent->mParent;
   }

   // set the color
   ColorI fontColor = mProfile->mFontColor;
   if(node->isSelected())
      fontColor = mProfile->mFontColorHL;
   else if(node->isGroup() && node->hasChildItem())
      fontColor.set(128, 0, 0);
   else if(!node->isGroup())
      fontColor.set(0, 0, 128);
   
   dglSetBitmapModulation(fontColor); //node->isSelected() ? mProfile->mFontColorHL : mProfile->mFontColor);
   dglDrawText(mProfile->mFont, Point2I(offset.x + mTxtOffset + mTabSize * (mNodeList[cell.y]->mTab + 1), offset.y), mNodeList[cell.y]->mName);
}
