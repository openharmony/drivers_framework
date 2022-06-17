import time
import os
import easyocr
import pytest
from PIL import Image
from selenium import webdriver
from selenium.webdriver import ActionChains
from public_moudle.Base import Base
import warnings
warnings.filterwarnings("ignore", category=UserWarning)

class Test_HcsTool:
    driver = webdriver.Chrome()
    test = Base(driver)
    url = test.getfiledir()

    def setup_class(self):
        #Determine whether the index.html file in the local project can be opened and used before executing the use case
        filelist = os.listdir('../../dist')
        if len(filelist) < 2:
            os.chdir('../../')
            os.system("python build.py")
        else:
            return

    def setup(self):
        #Open a web page in your browser
        self.driver.get(self.url)
        time.sleep(1)

    def teardown(self):
        #Data cleaning
        print('Clear cookies and exit the login state')
        self.driver.delete_all_cookies()
        self.driver.refresh()
        time.sleep(1)

    def teardown_class(self):
        #When the use case is complete, close the browser
        self.driver.quit()

    def test_rootselfattr(self):
        el = self.test.findElement(('id', 'visual_area'))
        loc = self.test.get_canvas_point_position('./Program_pictures/rootselfattr.png', 'root')
        self.test.click_canvas_point(el, loc[0], loc[1])
        time.sleep(2)
        rootname = self.driver.find_element_by_class_name('att_title').text
        assert rootname == 'root'
        nodetype = self.driver.find_element_by_xpath('/html/body/div/select/option[1]').text
        assert nodetype == '数据类不继承'   #The node type is data class and does not inherit
        result = self.test.get_attribute(('id', 'node_type'), 'disabled')
        assert result == 'true'  #The node type cannot be modified
        nodename = self.test.get_attribute(('id', 'name'), 'value')
        assert nodename == 'root'   #The node name is root
        nameedit = self.test.get_attribute(('id', 'name'), 'disabled')
        assert nameedit == 'true'  #The node name cannot be modified
        self.driver.refresh()
        time.sleep(1)
        self.test.add_node('root')  # add node
        time.sleep(0.5)
        self.driver.refresh()  #Refresh the page
        time.sleep(1)
        self.test.add_attr('root')  # add attr
        time.sleep(0.5)
        self.driver.refresh()
        time.sleep(1)
        self.test.click_canvas_by_text('root')  #click root
        self.test.click(('id','delete'))  #click delete button
        path = './Program_pictures/rootselfattr1.png'
        self.driver.get_screenshot_as_file(path)
        Image.open(path).crop((0.8 * 929, 0, 929, 0.3 * 888)).save(path)
        reader = easyocr.Reader(['ch_sim', 'en'], verbose=False)
        nodecount = reader.readtext(path)
        assert len(nodecount) == 1  #Gets the number of elements in the property edit area

    def test_addnodedata(self):
        time.sleep(0.5)
        self.test.add_node('root')
        time.sleep(1)
        topname = self.driver.find_element_by_class_name('att_title').text
        assert topname == 'node_1'
        nodetype = self.driver.find_element_by_xpath('/html/body/div/select/option[1]').text
        assert nodetype == '数据类不继承'
        self.test.sendkeys(('id','name'),'nodeceshi')
        time.sleep(0.5)
        flag = self.test.node_in_kuang('./Program_pictures/addnodedata1.png','nodeceshi')
        assert flag is True
        self.test.click(('id','add_child_node'))  #add node
        time.sleep(0.5)
        self.test.click(('id','add_child_attr'))  #add attr
        time.sleep(0.5)
        self.test.click_canvas_by_text('nodeceshi')  #delete node_1(nodeceshi)
        self.test.click(('id','delete'))
        deletnode = self.test.node_in_kuang('./Program_pictures/addnodedata2.png', 'nodeceshi')
        assert deletnode is False

    def test_datamodifycopy(self):
        self.test.add_node('root')   #add & click node
        time.sleep(0.5)
        self.driver.find_element_by_xpath('/html/body/div/select/option[2]').click()   #change node type copy
        time.sleep(0.5)
        result = self.test.get_attribute(('id', 'node_type'), 'disabled')  # node type cannot modified
        assert result == 'true'
        copyflag = self.test.kuang_contain_text('./Program_pictures/datamodifycopy1.png',"复制目标没找到")
        assert copyflag is True
        self.test.click(('id','change_target'))  #click copy button
        time.sleep(0.5)
        selecttarget = self.test.kuang_contain_text('./Program_pictures/datamodifycopy2.png', "点击选")
        assert selecttarget is True
        canceltarget = self.test.kuang_contain_text('./Program_pictures/datamodifycopy3.png', "取消选择")
        assert canceltarget is True
        self.test.sendkeys(('id', 'name'), 'hhhh')  #modify node name
        time.sleep(0.5)
        containhhhh = self.test.kuang_contain_text('./Program_pictures/datamodifycopy4.png','hhhh')
        assert containhhhh is True

    def test_copyself(self):
        el = self.test.findElement(('id', 'visual_area'))
        nodeloc = self.test.add_node('root')   #add & click node
        time.sleep(0.5)
        self.driver.find_element_by_xpath('/html/body/div/select/option[2]').click() #change node type copy
        time.sleep(0.5)
        self.test.click(('id', 'change_target')) #click copy button
        time.sleep(0.5)
        loc = self.test.get_obtain_point_position('./Program_pictures/copyself1.png','点击选')
        self.test.click_canvas_point(el,650,700)
        time.sleep(0.5)
        loc1 = self.test.get_obtain_point_position('./Program_pictures/copyself2.png', '点击选')
        assert loc[0] != loc1[0] and loc[1] != loc1[1]
        self.test.sendkeys(('id', 'name'), 'hhhh')  # modify node name
        time.sleep(0.5)
        self.test.click_canvas_point(el,nodeloc[0],nodeloc[1])  #copy self
        time.sleep(0.5)
        selecttarget = self.test.kuang_contain_text('./Program_pictures/copyself3.png', "点击选")
        assert selecttarget is False
        canceltarget = self.test.kuang_contain_text('./Program_pictures/copyself4.png', "取消选择")
        assert canceltarget is False
        copyflag = self.test.kuang_contain_text('./Program_pictures/copyself5.png', "循环复制")
        assert copyflag is True
        unknowexit = self.test.node_in_kuang('./Program_pictures/copyself6.png','unknow')
        assert unknowexit is False

    def test_modiycopy(self):
        el = self.test.findElement(('id', 'visual_area'))
        nodeloc = self.test.add_node('root')  #add & click node
        time.sleep(0.5)
        self.driver.find_element_by_xpath('/html/body/div/select/option[2]').click()  #change node type copy
        self.test.click(('id', 'change_target'))   #click copy button
        self.test.click_canvas_point(el, nodeloc[0], nodeloc[1])  #copy self
        time.sleep(0.5)
        self.test.click(('id', 'change_target'))  # clck copy button
        time.sleep(1)
        self.test.click_canvas_by_text('nodeaa')  #copy nodeaa
        time.sleep(1)
        text = self.driver.find_element_by_id('change_target').text
        assert text == 'nodeaa'

    def test_modifycopyname(self):
        time.sleep(1)
        nodeloc = self.test.add_node('root')   #add & click node
        time.sleep(0.5)
        self.driver.find_element_by_xpath('/html/body/div/select/option[2]').click()  #change node type copy
        self.test.click(('id', 'change_target'))  #click copy button
        self.test.click_canvas_by_text('nodeaa')  # copy nodeaa
        time.sleep(0.5)
        loc = self.test.get_canvas_point_position('./Program_pictures/modifycopyname1.png','nodeaa')
        el = self.test.findElement(('id', 'visual_area'))
        self.test.click_canvas_point(el,loc[0],loc[1])  #click nodeaa
        time.sleep(0.5)
        self.test.sendkeys(('id','name'),'nodeff')  #modify nodeaa name
        text = self.test.get_attribute(('id','name'),'value')
        assert text == 'nodeff'
        time.sleep(0.5)
        # copyflag = self.test.kuang_contain_text('./Program_pictures/modifycopyname2.png', "复制目标没找到")
        # assert copyflag is True
        self.test.click_canvas_point(el,nodeloc[0],nodeloc[1])  #click node_1 again
        self.test.click(('id', 'change_target'))   # click copy button
        time.sleep(0.5)
        self.test.click_canvas_point(el,loc[0],loc[1])   #copy nodeff  again
        time.sleep(0.5)
        copyname = self.driver.find_element_by_id('change_target').text
        assert copyname == 'nodeff'

    def test_copyattr(self):
        time.sleep(1)
        self.test.add_node('root')   #add & click node
        time.sleep(0.5)
        self.driver.find_element_by_xpath('/html/body/div/select/option[2]').click()  #change node type copy
        self.test.click(('id', 'change_target'))  #click copy button
        self.test.click_canvas_by_text('extstring')   #copy  extstring
        time.sleep(0.5)
        cancelselect = self.test.kuang_contain_text('./Program_pictures/copyattr.png', "取消选择")
        assert cancelselect is True

    def test_cancelcopy(self):
        self.test.add_node('root')  #add & click node
        time.sleep(0.5)
        self.driver.find_element_by_xpath('/html/body/div/select/option[2]').click()  #change node type copy
        self.test.click(('id', 'change_target')) #click copy button
        self.test.click_canvas_by_text('取消选择')  # click cancel copy button
        time.sleep(0.5)
        cancelselect = self.test.kuang_contain_text('./Program_pictures/cancelselect1.png', "取消选择")
        assert cancelselect is False
        selecttarget = self.test.kuang_contain_text('./Program_pictures/cancelselect2.png', "点击选")
        assert selecttarget is False
        copyflag = self.test.kuang_contain_text('./Program_pictures/cancelselect3.png', "复制目标没找到")
        assert copyflag is True

    def test_reference(self):
        self.test.add_node('root')   #add & click node
        time.sleep(0.5)
        self.driver.find_element_by_xpath('/html/body/div/select/option[3]').click()  # change node type reference
        time.sleep(0.5)
        result = self.test.get_attribute(('id', 'node_type'), 'disabled')  # node type cannot modified
        assert result == 'true'
        referenceflag = self.test.kuang_contain_text('./Program_pictures/reference1.png', "目标没找到")
        assert referenceflag is True
        self.test.click(('id', 'change_target'))  # click reference button
        time.sleep(0.5)
        selecttarget = self.test.kuang_contain_text('./Program_pictures/reference2.png', "点击选")
        assert selecttarget is True
        canceltarget = self.test.kuang_contain_text('./Program_pictures/reference3.png', "取消选择")
        assert canceltarget is True

    def test_referenceself(self):
        el = self.test.findElement(('id', 'visual_area'))
        nodeloc = self.test.add_node('root')  #add & click node
        time.sleep(0.5)
        self.driver.find_element_by_xpath('/html/body/div/select/option[3]').click()  #change node type reference
        time.sleep(0.5)
        self.test.click(('id', 'change_target'))  # click reference button
        time.sleep(0.5)
        loc = self.test.get_obtain_point_position('./Program_pictures/referenceself1.png', '点击选择')
        self.test.click_canvas_point(el, 650, 700)
        time.sleep(0.5)
        loc1 = self.test.get_obtain_point_position('./Program_pictures/referenceself2.png', '点击选择')
        assert loc[0] != loc1[0] and loc[1] != loc1[1]
        self.test.sendkeys(('id', 'name'), 'hhhh')  # modify node name
        time.sleep(0.5)
        self.test.click_canvas_point(el, nodeloc[0], nodeloc[1])  # refence self
        time.sleep(0.5)
        selecttarget = self.test.kuang_contain_text('./Program_pictures/referenceself3.png', "点击选择")
        assert selecttarget is False
        canceltarget = self.test.kuang_contain_text('./Program_pictures/referenceself4.png', "取消选择")
        assert canceltarget is False
        copyflag = self.test.kuang_contain_text('./Program_pictures/referenceself5.png', "环引用")
        assert copyflag is True
        text  = self.driver.find_element_by_id('change_target').text
        assert text == 'hhhh'

    def test_modifyreference(self):
        el = self.test.findElement(('id', 'visual_area'))
        nodeloc = self.test.add_node('root')   #add & click node
        time.sleep(0.5)
        self.driver.find_element_by_xpath('/html/body/div/select/option[3]').click()  #change node type to reference
        self.test.click(('id', 'change_target'))  # click reference button
        self.test.click_canvas_point(el, nodeloc[0], nodeloc[1])  # refence self
        time.sleep(0.5)
        self.test.click(('id', 'change_target'))  # click reference button
        time.sleep(1)
        self.test.click_canvas_by_text('nodeaa')  # reference nodeaa
        time.sleep(1)
        text = self.driver.find_element_by_id('change_target').text
        assert text == 'nodeaa'

    def test_modifyrefename(self):
        nodeloc = self.test.add_node('root') #add & click node
        time.sleep(0.5)
        self.driver.find_element_by_xpath('/html/body/div/select/option[3]').click()  #change node type to reference
        self.test.click(('id', 'change_target')) # click reference button
        self.test.click_canvas_by_text('nodeaa')  # reference nodeaa
        time.sleep(0.5)
        loc = self.test.get_canvas_point_position('./Program_pictures/modifyrefename1.png', 'nodeaa')
        el = self.test.findElement(('id', 'visual_area'))
        self.test.click_canvas_point(el, loc[0], loc[1])  # click nodeaa
        time.sleep(0.5)
        self.test.sendkeys(('id', 'name'), 'nodeff')  # modify nodeaa  name
        text = self.test.get_attribute(('id', 'name'), 'value')
        assert text == 'nodeff'
        time.sleep(0.5)
        # refeflag = self.test.kuang_contain_text('./Program_pictures/modifyrefename2.png', "目标没找到")
        # assert refeflag is True
        self.test.click_canvas_point(el, nodeloc[0], nodeloc[1])  # click node_1
        self.test.click(('id', 'change_target'))  # click reference button
        time.sleep(0.5)
        self.test.click_canvas_point(el, loc[0], loc[1])  # reference nodeff
        time.sleep(0.5)
        refename = self.driver.find_element_by_id('change_target').text
        assert refename == 'nodeff'

    def test_refeattr(self):
        print("")
        self.test.add_node('root')  #add & click node
        time.sleep(0.5)
        self.driver.find_element_by_xpath('/html/body/div/select/option[3]').click()  #change node type reference
        self.test.click(('id', 'change_target'))  # click reference button
        self.test.click_canvas_by_text('extstring')  # reference extstring
        time.sleep(0.5)
        cancelselect = self.test.kuang_contain_text('./Program_pictures/refeattr.png', "取消选择")
        assert cancelselect is True

    def test_cancelrefe(self):
        self.test.add_node('root')  #add & click node
        time.sleep(0.5)
        self.driver.find_element_by_xpath('/html/body/div/select/option[3]').click()   #change node type reference
        self.test.click(('id', 'change_target'))  # click reference button
        self.test.click_canvas_by_text('取消选择')  # click cancel reference button
        time.sleep(0.5)
        cancelselect = self.test.kuang_contain_text('./Program_pictures/cancelrefe1.png', "取消选择")
        assert cancelselect is False
        selecttarget = self.test.kuang_contain_text('./Program_pictures/cancelrefe2.png', "点击选")
        assert selecttarget is False
        copyflag = self.test.kuang_contain_text('./Program_pictures/cancelrefe3.png', "目标没找到")
        assert copyflag is True

    def test_delete(self):
        self.test.add_node('root')  #add & click node
        time.sleep(0.5)
        self.driver.find_element_by_xpath('/html/body/div/select/option[4]').click()  #change node type delete
        time.sleep(0.5)
        result = self.test.get_attribute(('id', 'node_type'), 'disabled')  # node type cannot modified
        assert result == 'true'
        self.test.click(('id', 'delete'))  # click delete button
        time.sleep(0.5)
        deleteexit = self.test.kuang_contain_text('./Program_pictures/delete.png', "delete")
        assert deleteexit is False

    def test_templete(self):
        self.test.add_node('root')   #add & click node
        time.sleep(0.5)
        self.driver.find_element_by_xpath('/html/body/div/select/option[5]').click()   #change node type templete
        time.sleep(0.5)
        result = self.test.get_attribute(('id', 'node_type'), 'disabled') # node type cannot modified
        assert result == 'true'
        time.sleep(0.5)
        self.test.click(('id','add_child_node'))
        self.test.click(('id','add_child_attr'))

    def test_inheritance(self):
        self.test.add_node('root')  #add & click node
        time.sleep(0.5)
        # Change the node type to inheritance class
        self.driver.find_element_by_xpath('/html/body/div/select/option[6]').click()
        time.sleep(0.5)
        result = self.test.get_attribute(('id', 'node_type'), 'disabled')  # node type cannot modified
        assert result == 'true'
        referenceflag = self.test.kuang_contain_text('./Program_pictures/inheritance1.png', "不到继承目标")
        assert referenceflag is True
        self.test.click(('id', 'change_target'))   # Click the Inheritance button
        time.sleep(0.5)
        selecttarget = self.test.kuang_contain_text('./Program_pictures/inheritance2.png', "点击选")
        assert selecttarget is True
        canceltarget = self.test.kuang_contain_text('./Program_pictures/inheritance3.png', "取消选择")
        assert canceltarget is True

    def test_inherself(self):
        el = self.test.findElement(('id', 'visual_area'))
        nodeloc = self.test.add_node('root')  #add & click node
        time.sleep(0.5)
        # Change the node type to inheritance class
        self.driver.find_element_by_xpath('/html/body/div/select/option[6]').click()
        time.sleep(0.5)
        self.test.click(('id', 'change_target'))    # Click the Inheritance button
        time.sleep(0.5)
        loc = self.test.get_obtain_point_position('./Program_pictures/inherself1.png', '点击选择')
        self.test.click_canvas_point(el, 650, 700)
        time.sleep(0.5)
        loc1 = self.test.get_obtain_point_position('./Program_pictures/inherself2.png', '点击选择')
        assert loc[0] != loc1[0] and loc[1] != loc1[1]
        self.test.sendkeys(('id', 'name'), 'hhhh')
        time.sleep(0.5)
        self.test.click_canvas_point(el, nodeloc[0], nodeloc[1])  #Inherit  self
        time.sleep(0.5)
        selecttarget = self.test.kuang_contain_text('./Program_pictures/inherself3.png', "点击选择")
        assert selecttarget is False
        canceltarget = self.test.kuang_contain_text('./Program_pictures/inherself4.png', "取消选择")
        assert canceltarget is False
        text  = self.driver.find_element_by_id('change_target').text
        assert text == 'hhhh'

    def test_modifyinher(self):
        el = self.test.findElement(('id', 'visual_area'))
        nodeloc = self.test.add_node('root')  #add & click node
        time.sleep(0.5)
        # Change the node type to inheritance class
        self.driver.find_element_by_xpath('/html/body/div/select/option[6]').click()
        self.test.click(('id', 'change_target'))   # Click the Inheritance button
        self.test.click_canvas_point(el, nodeloc[0], nodeloc[1])  #Inherit  self
        time.sleep(0.5)
        self.test.click(('id', 'change_target'))   # Click the Inheritance button
        time.sleep(1)
        self.test.click_canvas_by_text('nodeaa')  # Inherit nodeaa
        time.sleep(1)
        text = self.driver.find_element_by_id('change_target').text
        assert text == 'nodeaa'

    def test_modifyinhername(self):
        self.test.click_canvas_by_text('nodeaa')
        time.sleep(0.5)
        self.driver.find_element_by_xpath('/html/body/div/select/option[5]').click()
        time.sleep(1)
        nodeloc = self.test.add_node('root')  # add && click node
        time.sleep(0.5)
        # Change the node type to inheritance class
        self.driver.find_element_by_xpath('/html/body/div/select/option[6]').click()
        self.test.click(('id', 'change_target'))   # Click the Inheritance button
        self.test.click_canvas_by_text('templete nodeaa')  # Inherits an existing attribute nodeaa
        time.sleep(0.5)
        loc = self.test.get_canvas_point_position('./Program_pictures/modifyinhername1.png', 'templete nodeaa')
        el = self.test.findElement(('id', 'visual_area'))
        self.test.click_canvas_point(el, loc[0], loc[1])  # click nodeaa
        time.sleep(0.5)
        self.test.sendkeys(('id', 'name'), 'nodeff')  # modify nodeaa name
        text = self.test.get_attribute(('id', 'name'), 'value')
        assert text == 'nodeff'      #Verify that the node name is successfully changed
        time.sleep(0.5)
        refeflag = self.test.kuang_contain_text('./Program_pictures/modifyinhername2.png', "不能继承数据类节点")
        assert refeflag is True
        self.test.click_canvas_point(el, nodeloc[0], nodeloc[1])  # Click the node_1 node again
        self.test.click(('id', 'change_target'))  # Click the Inheritance button
        time.sleep(0.5)
        self.test.click_canvas_point(el, loc[0], loc[1])  # Inherit the nodeff node again
        time.sleep(0.5)
        refename = self.driver.find_element_by_id('change_target').text
        assert refename == 'nodeff'

    def test_inheattr(self):
        self.test.add_node('root')  # add && click node
        time.sleep(0.5)
        # Change the node type to inheritance class
        self.driver.find_element_by_xpath('/html/body/div/select/option[6]').click()
        self.test.click(('id', 'change_target'))  # Click the Inheritance button
        self.test.click_canvas_by_text('extstring')  # Inherits an existing attribute node
        time.sleep(0.5)
        cancelselect = self.test.kuang_contain_text('./Program_pictures/inheattr.png', "取消选择")
        assert cancelselect is True   #The deselect button still exists on the canvas

    def test_cancelinher(self):
        self.test.add_node('root')  # add && click node
        time.sleep(0.5)
        # Change the node type to inheritance class
        self.driver.find_element_by_xpath('/html/body/div/select/option[6]').click()
        self.test.click(('id', 'change_target'))  # Click the Inheritance button
        self.test.click_canvas_by_text('取消选择')  # click cancal select
        time.sleep(0.5)
        cancelselect = self.test.kuang_contain_text('./Program_pictures/cancelinher1.png', "取消选择")
        assert cancelselect is False   #The deselect button still exists on the canvas
        selecttarget = self.test.kuang_contain_text('./Program_pictures/cancelinher2.png', "点击选")
        assert selecttarget is False   #Canvas without prompt click to select target text
        copyflag = self.test.kuang_contain_text('./Program_pictures/cancelinher3.png', "不到继承目标")
        assert copyflag is True  #Canvas hint could not find inheritance target

    def test_addattr(self):
        self.test.add_attr('root')   #add attr
        attname = self.driver.find_element_by_class_name('att_title').text
        assert attname == 'attr_1'
        self.test.sendkeys(('id','name'),'addattr')   #modify attr name
        flag = self.test.kuang_contain_text('./Program_pictures/addattr1.png','addattr')
        assert flag == True   #Verify that the property name was successfully modified
        text = self.driver.find_element_by_xpath('/html/body/div/select/option[1]').text  #get attr type
        assert text == '整数'
        attrvalue = self.test.get_attribute(('id','value'),'value') # get attr values
        assert attrvalue == '0'
        self.test.sendkeys(('id','value'),3)
        modifyvalue = self.test.get_attribute(('id', 'value'), 'value') # modify attr values
        assert modifyvalue == '3'
        self.test.click(('id','delete'))    #delete attr
        deleteattr = self.test.kuang_contain_text('./Program_pictures/addattr2.png', 'addattr')
        assert deleteattr == False  #Verifying a successful deletion

    def test_modifystring(self):
        self.test.add_attr('root')
        time.sleep(0.5)
        self.driver.find_element_by_xpath('/html/body/div/select/option[2]').click()   #change attr type is string
        time.sleep(0.5)
        stringtext = self.test.get_attribute(('id','value'),'value')  # get attr values
        assert stringtext == ''
        self.test.sendkeys(('id','value'),'hhh***你好5678')  # modify attr values
        stringattr = self.test.kuang_contain_text('./Program_pictures/modifystring.png', '你好5678')
        assert stringattr == True   # Verify modify successfully

    def test_modifyarray(self):
        self.test.add_attr('root')
        time.sleep(0.5)
        self.driver.find_element_by_xpath('/html/body/div/select/option[3]').click()  # change attr type is array
        time.sleep(0.5)
        stringtext = self.test.get_attribute(('id', 'value'), 'value')  # get attr values
        assert stringtext == ''
        self.test.sendkeys(('id', 'value'), r"'ceshi',1,2,1,,,,,4")  # modify attr values
        arrayattr = self.test.kuang_contain_text('./Program_pictures/modifyarray.png', "9]0,1,2,1,0,0,0,0,4")
        assert arrayattr == True  # Verify modify successfully

    def test_modifybool(self):
        self.test.add_attr('root')
        time.sleep(0.5)
        self.driver.find_element_by_xpath('/html/body/div/select/option[4]').click()  # change attr type is bool
        time.sleep(0.5)
        booltext = self.test.get_attribute(('id', 'value'), 'value')  # Get property values
        assert booltext == 'true'
        self.driver.find_element_by_xpath('//*[@id="value"]/option[2]').click()  # modify attr false
        boolattr = self.test.kuang_contain_text('./Program_pictures/modifybool.png',"alse")
        assert boolattr == True  # Verify the modification

    def test_modifurefe(self):
        self.test.add_attr('root')
        time.sleep(0.5)
        self.driver.find_element_by_xpath('/html/body/div/select/option[5]').click()  # change attr type is reference
        time.sleep(0.5)
        refeattr = self.test.kuang_contain_text('./Program_pictures/modifurefe1.png', "目标")
        assert refeattr == True # Failed to find reference target in validation canvas
        unknowattr = self.test.kuang_contain_text('./Program_pictures/modifurefe2.png', "Unknow")
        assert unknowattr == True  # The attribute node name in the validation canvas contains unknow
        self.test.click(('id','change_target'))  #click reference button
        time.sleep(0.5)
        # Verify that the property node name in the canvas was changed successfully
        selectattr = self.test.kuang_contain_text('./Program_pictures/modifurefe3.png', "点击选")
        assert selectattr == True
        # The deselect button appears in the lower right corner of the validation canvas
        cancelselect = self.test.kuang_contain_text('./Program_pictures/modifurefe4.png', "取消选择")
        assert cancelselect == True
        loc = self.test.get_obtain_point_position('./Program_pictures/modifurefe5.png','extint')
        el = self.test.findElement(('id', 'visual_area'))
        self.test.click_canvas_point(el,loc[0],loc[1])   #click extint
        # The deselect button appears in the lower right corner of the validation canvas
        clickattr = self.test.kuang_contain_text('./Program_pictures/modifurefe6.png', "取消选择")
        assert clickattr == True
        nodeloc = self.test.get_obtain_point_position('./Program_pictures/modifurefe7.png',
                                                      'nodeaa')  # get nodeaa loction
        el = self.test.findElement(('id', 'visual_area'))
        self.test.click_canvas_point(el, nodeloc[0], nodeloc[1])  # click nodeaa
        time.sleep(2)
        text = self.driver.find_element_by_id('change_target').text   #get reference button name
        assert text == 'nodeaa'

    def test_modifydelete(self):
        self.test.add_attr('root')
        time.sleep(0.5)
        self.driver.find_element_by_xpath('/html/body/div/select/option[6]').click()  # change attr type is delete
        time.sleep(0.5)
        before = self.test.kuang_contain_text('./Program_pictures/modifydelete1.png', "elete")
        assert before == True
        self.test.click(('id','delete'))   #click delete button
        after = self.test.kuang_contain_text('./Program_pictures/modifydelete2.png', "elete")
        assert after == False

    def test_draginkuang(self):
        beforedrag = self.test.get_canvas_point_position('./Program_pictures/draginkuang1.png', 'root')
        el = self.test.findElement(('id','visual_area'))
        size = self.driver.get_window_size()
        height = size.get('height')
        actions = ActionChains(self.driver)
        actions.click_and_hold(el).perform()
        tracks = [num*50 for num in range(6,19)]
        for i in tracks:
            actions.w3c_actions.pointer_action.source.create_pointer_move(50,i,height-i)
        for i in tracks[::-1]:
            actions.w3c_actions.pointer_action.source.create_pointer_move(50, i, 600)
        actions.release(el)
        time.sleep(3)
        actions.move_to_element_with_offset(el,500,500).click()
        afterdrag = self.test.get_canvas_point_position('./Program_pictures/draginkuang2.png', 'root')
        actions.perform()
        assert beforedrag[0] == afterdrag[0] and beforedrag[1] == afterdrag[1]

    def test_dragoutkuang(self):
        beforedrag = self.test.get_canvas_point_position('./Program_pictures/dragoutkuang1.png', 'root')
        el = self.test.findElement(('id', 'visual_area'))
        actions = ActionChains(self.driver)
        actions.click_and_hold(el)
        lst = [(300, 600), (350, 550), (400, 500), (450, 450), (500, 400), (550, 350), (600, 300), (650, 250),
               (700, 200), (750, 150), (800, 100), (850, 15)]
        for i in lst:
            actions.w3c_actions.pointer_action.source.create_pointer_move(50, i[0], i[1])
        el1 = self.driver.find_element_by_class_name('att_title')
        actions.move_to_element(el1).click()
        loc = self.test.get_canvas_point_position('./Program_pictures/dragoutkuang2.png', 'root')
        lst1 = [(850, 15), (750, 150), (600, 300), (450, 450),(300,600),(150,750)]
        for i in lst1:
            actions.w3c_actions.pointer_action.source.create_pointer_move(50, i[0], i[1])
        actions.move_to_element_with_offset(el, 700, 600).click()
        afterdrag = self.test.get_canvas_point_position('./Program_pictures/dragoutkuang3.png', 'root')
        actions.perform()
        assert beforedrag[0] == afterdrag[0] and beforedrag[1] == afterdrag[1]

    def test_dragel(self):
        loc = self.test.get_canvas_point_position('./Program_pictures/dragel.png','root')
        el = self.test.findElement(('id', 'visual_area'))
        self.test.click_canvas_point(el,loc[0],loc[1])
        actions = ActionChains(self.driver)
        actions.move_to_element_with_offset(el,loc[0],loc[1]).click_and_hold()
        lst = [(int(loc[0]), int(loc[1])), (100, 100), (300, 300), (400, 400), (500, 500), (600, 600)]
        for i in lst:
            actions.w3c_actions.pointer_action.source.create_pointer_move(50, i[0], i[1])
        actions.perform()
        time.sleep(2)

    def test_attrcopyself(self):
        loc = self.test.get_canvas_point_position('./Program_pictures/attrcopyself1.png', 'extstring')
        el = self.test.findElement(('id', 'visual_area'))
        self.test.click_canvas_point(el,loc[0],loc[1])
        self.test.canvas_copy()
        time.sleep(0.5)
        copyself = self.test.kuang_contain_text('./Program_pictures/attrcopyself2.png','己复制')
        assert copyself is True
        cancelcopy = self.test.kuang_contain_text('./Program_pictures/attrcopyself3.png', '取消复制')
        assert cancelcopy is True
        beforecopy = self.test.text_count('./Program_pictures/attrcopyself4.png', 'string')
        self.test.canvas_paste()
        time.sleep(2)
        aftercopy = self.test.text_count('./Program_pictures/attrcopyself5.png', 'string')
        assert aftercopy == beforecopy+1
        # beforeeditname = self.test.text_count('./Program_pictures/attrcopyself6.png', '重名')
        # assert beforeeditname == 1
        self.test.click_canvas_point(el,loc[0],loc[1]+25)
        time.sleep(1)
        self.test.sendkeys(('id', 'name'), 'string')
        time.sleep(2)
        aftereditname = self.test.text_count('./Program_pictures/attrcopyself7.png', '重名')
        assert aftereditname == 0

    def test_copysamelevelnode(self):
        self.test.click_canvas_by_text('extstring')  # click extstring
        self.test.canvas_copy()  # keyboard copy extstring
        loc = self.test.get_canvas_point_position('./Program_pictures/copysamelevelnode1.png',
                                                  'nodebb')   #get nodebb location
        el = self.test.findElement(('id', 'visual_area'))  #get canvas element
        self.test.click_canvas_point(el, loc[0], loc[1])   #click nodebb
        self.test.canvas_paste()  # keyboard paste extstring
        aftercopy = self.test.text_count('./Program_pictures/copysamelevelnode2.png', 'string')
        assert aftercopy >= 2
        flag = self.test.kuang_contain_text('./Program_pictures/copysamelevelnode3.png', '重名')
        assert flag is False

    def test_copysamelevelattr(self):
        self.test.click_canvas_by_text('extstring')  # click extstring
        self.test.canvas_copy()   # keyboard copy extstring
        loc = self.test.get_obtain_point_position('./Program_pictures/copysamelevelattr1.png',
                                                  'extint')   # get extint location
        el = self.test.findElement(('id', 'visual_area'))  # get canvas element
        self.test.click_canvas_point(el, loc[0], loc[1])  # click extint
        self.test.canvas_paste()   # keyboard paste extstring
        aftercopy = self.test.text_count('./Program_pictures/copysamelevelattr2.png', 'string')
        assert aftercopy == 2
        # flag = self.test.kuang_contain_text('./Program_pictures/copysamelevelattr3.png', '重名')
        # assert flag is True

    def test_copynextattr(self):
        self.test.click_canvas_by_text('extstring')  # click extstring
        self.test.canvas_copy()  # keyboard copy extstring
        loc = self.test.get_obtain_point_position('./Program_pictures/copynextattr1.png',
                                                  'childatt')  # get childatt location
        el = self.test.findElement(('id', 'visual_area'))   #get canvas element
        self.test.click_canvas_point(el, loc[0], loc[1])  # click childatt
        self.test.canvas_paste()  # keyboard paste extstring
        aftercopy = self.test.text_count('./Program_pictures/copynextattr2.png', 'string')
        assert aftercopy >= 2
        flag = self.test.kuang_contain_text('./Program_pictures/copynextattr3.png', '重名')
        assert flag is False

    def test_copynextnode(self):
        self.test.click_canvas_by_text('extstring')  # click extstring
        self.test.canvas_copy()  # keyboard copy extstring
        loc = self.test.get_obtain_point_position('./Program_pictures/copynextnode1.png',
                                                  'childnode')  # get childnode location
        el = self.test.findElement(('id', 'visual_area'))  #get canvas element
        self.test.click_canvas_point(el, loc[0], loc[1])  # click childnode
        self.test.canvas_paste() # keyboard paste extstring
        aftercopy = self.test.text_count('./Program_pictures/copynextnode2.png', 'string')
        assert aftercopy >= 2
        flag = self.test.kuang_contain_text('./Program_pictures/copynextnode3.png', '重名')
        assert flag is False

    def test_attrcopyroot(self):
        self.test.click_canvas_by_text('extstring')  # click extstring
        self.test.canvas_copy()  # keyboard copy extstring
        loc = self.test.get_obtain_point_position('./Program_pictures/attrcopyroot1.png',
                                                  'root')  # get root location
        el = self.test.findElement(('id', 'visual_area'))  # get canvas element
        self.test.click_canvas_point(el, loc[0], loc[1])  # click root
        self.test.canvas_paste()  # keyboard paste extstring
        aftercopy = self.test.text_count('./Program_pictures/attrcopyroot2.png', 'string')
        assert aftercopy == 2
        # repeatname = self.test.text_count('./Program_pictures/attrcopyroot3.png', '重名')
        # assert repeatname == 1

    def test_attrcancelcopy(self):
        self.test.click_canvas_by_text('extstring')  # click extstring attr node
        self.test.canvas_copy()  # key copy node
        loc = self.test.get_obtain_point_position('./Program_pictures/attrcancelcopy1.png',
                                                  '取消复制') #get cancel copy location
        el = self.test.findElement(('id', 'visual_area'))  # get canvas element
        self.test.click_canvas_point(el, loc[0], loc[1])  # click cancl copy
        self.test.canvas_paste()  # key paste node
        aftercopy = self.test.text_count('./Program_pictures/attrcancelcopy2.png', 'string')
        assert aftercopy == 1
        flag = self.test.kuang_contain_text('./Program_pictures/attrcancelcopy3.png', '重名')
        assert flag is False
        copyself = self.test.kuang_contain_text('./Program_pictures/attrcancelcopy4.png', '己复制')
        assert copyself is False
        cancelcopy = self.test.kuang_contain_text('./Program_pictures/attrcancelcopy5.png', '取消复制')
        assert cancelcopy is False

    def test_nodecopynode(self):
        self.test.click_canvas_by_text('nodebb')  # click nodebb
        self.test.canvas_copy()  # keyboard copy node
        loc = self.test.get_canvas_point_position('./Program_pictures/nodecopynode1.png',
                                                  'nodeaa')  # get nodeaa location
        el = self.test.findElement(('id', 'visual_area'))  # get canvas element
        self.test.click_canvas_point(el, loc[0], loc[1])  # click nodeaa
        self.test.canvas_paste()  # keyboar paste node
        aftercopy = self.test.text_count('./Program_pictures/nodecopynode2.png', 'nodebb')
        assert aftercopy >= 2
        flag = self.test.kuang_contain_text('./Program_pictures/nodecopynode3.png', '重名')
        assert flag is False

    def test_nodecopyattr(self):
        self.test.click_canvas_by_text('nodebb')  # click nodebb
        self.test.canvas_copy()  # keyboard copy node
        loc = self.test.get_obtain_point_position('./Program_pictures/nodecopyattr1.png',
                                                  'extstring')   # get extstring location
        el = self.test.findElement(('id', 'visual_area'))  # get canvas element
        self.test.click_canvas_point(el, loc[0], loc[1])  # click extstring attr node
        self.test.canvas_paste()  # keyboard paste node
        aftercopy = self.test.text_count('./Program_pictures/nodecopyattr2.png', 'nodebb')
        assert aftercopy == 2
        # flag = self.test.kuang_contain_text('./Program_pictures/nodecopyattr3.png', '重名')
        # assert flag is True

    def test_nodecopynextattr(self):
        self.test.click_canvas_by_text('nodebb')  # click nodebb
        self.test.canvas_copy()  # keyboard copy nodebb
        loc = self.test.get_obtain_point_position('./Program_pictures/nodecopynextattr1.png',
                                                  'childatt')  # get childatt location
        el = self.test.findElement(('id', 'visual_area'))  # get canvas element
        self.test.click_canvas_point(el, loc[0], loc[1])  # click childatt
        self.test.canvas_paste()  # keyboard paste nodebb
        aftercopy = self.test.text_count('./Program_pictures/nodecopynextattr2.png', 'nodebb')
        assert aftercopy >= 2
        flag = self.test.kuang_contain_text('./Program_pictures/nodecopynextattr3.png', '重名')
        assert flag is False

    def test_nodecopynextnode(self):
        self.test.click_canvas_by_text('nodebb')  # click nodebb
        self.test.canvas_copy()  # keyboard copy node
        loc = self.test.get_obtain_point_position('./Program_pictures/nodecopynextnode1.png',
                                                  'childnode')  # get childnode location
        el = self.test.findElement(('id', 'visual_area'))  # get canvas element
        self.test.click_canvas_point(el, loc[0], loc[1])  # click childnode
        self.test.canvas_paste()  # keyboard paste nodebb
        aftercopy = self.test.text_count('./Program_pictures/nodecopynextnode2.png', 'nodebb')
        assert aftercopy >= 2
        flag = self.test.kuang_contain_text('./Program_pictures/nodecopynextnode3.png', '重名')
        assert flag is False

    def test_nodecopyroot(self):
        self.test.click_canvas_by_text('nodebb')  # click nodebb
        self.test.canvas_copy()  # keyboard copy node
        loc = self.test.get_obtain_point_position('./Program_pictures/nodecopyroot1.png',
                                                  'oot')  # get root location
        el = self.test.findElement(('id', 'visual_area'))  # get canvas element
        self.test.click_canvas_point(el, loc[0], loc[1])  # click root
        self.test.canvas_paste()  # keyboard paste nodebb
        aftercopy = self.test.text_count('./Program_pictures/nodecopyroot2.png', 'nodebb')
        assert aftercopy >= 2
        # repeatname = self.test.text_count('./Program_pictures/nodecopyroot3.png', '重名')
        # assert repeatname == 1

    def test_nodecopyself(self):
        loc = self.test.get_canvas_point_position('./Program_pictures/nodecopyself1.png', 'nodebb')  #get nodebb location
        el = self.test.findElement(('id', 'visual_area'))  #get canvas element
        self.test.click_canvas_point(el, loc[0], loc[1])   #click nodebb
        self.test.canvas_copy()   #keyboard copy nodebb
        time.sleep(1)
        copyself = self.test.text_count('./Program_pictures/nodecopyself2.png', '已复制')
        assert copyself == 1
        cancelcopy = self.test.kuang_contain_text('./Program_pictures/nodecopyself3.png', '取消复制')
        assert cancelcopy is True
        beforecopy = self.test.text_count('./Program_pictures/nodecopyself4.png', 'nodebb')
        self.test.canvas_paste()   #keyboar paste nodebb
        time.sleep(2)
        aftercopy = self.test.text_count('./Program_pictures/nodecopyself5.png', 'nodebb')
        assert aftercopy == beforecopy + 1  #Verify that nodebb is added to the drawing box
        repeatname = self.test.text_count('./Program_pictures/nodecopyself6.png', '重名')
        assert repeatname == 0
        nodeaaloc = self.test.get_canvas_point_position('./Program_pictures/nodecopyself7.png', 'nodeaa')
        self.test.click_canvas_point(el, nodeaaloc[0], nodeaaloc[1])  #click nodeaa
        self.test.canvas_paste()  #paste nodebb onto nodeaa
        aftercopyagain = self.test.text_count('./Program_pictures/nodecopyself8.png', 'nodebb')
        assert aftercopyagain >= aftercopy + 2  # check paste new nodebb

