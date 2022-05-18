import time

import easyocr
import os
from PIL import Image
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.support.wait import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.common.action_chains import ActionChains

class Base:
    def __init__(self, driver):
        self.driver = driver
        self.timeout = 10
        self.t = 0.5

    def findElement(self, locator):
        if not isinstance(locator, tuple):
            print('The type of the locator parameter is tuple')
        else:
            ele = WebDriverWait(self.driver,
                                self.timeout,
                                self.t).until(EC.presence_of_element_located(locator))
            return ele

    #Get URL address
    def getfiledir(self):
        os.chdir('../../')
        curFile = os.getcwd()
        path = curFile.replace('\\', '/')
        url = 'file:///' + path + '/index.html'
        os.chdir('test/testsuits')
        return url

    #Enter text information
    def sendkeys(self, locator, text=''):
        ele = self.findElement(locator)
        ele.clear()
        ele.send_keys(text)

    def click(self, locator):
        ele = self.findElement(locator)
        ele.click()

    def get_attribute(self, locator, name):
        try:
            element = self.findElement(locator)
            return element.get_attribute(name)
        except BaseException:
            return ""

    def click_canvas_point(self,el,x,y):
        actions = ActionChains(self.driver)
        actions.move_to_element_with_offset(el, x, y).click().perform()

    def click_canvas_by_text(self,text):
        #text:Click on the text
        el = self.findElement(('id', 'visual_area'))
        loc = self.get_canvas_point_position('./Program_pictures/quanping.png',text)
        actions = ActionChains(self.driver)
        actions.move_to_element_with_offset(el, loc[0], loc[1]).click().perform()

    def canvas_copy(self):
        actions = ActionChains(self.driver)
        actions.key_down(Keys.CONTROL).send_keys('c').key_up(Keys.CONTROL).perform()

    def canvas_paste(self):
        actions = ActionChains(self.driver)
        actions.key_down(Keys.CONTROL).send_keys('v').key_up(Keys.CONTROL).perform()

    def get_canvas_point_position(self,path,text):
        #path:Storage address of screenshot   text:The text content to look up
        self.driver.get_screenshot_as_file(path)
        reader = easyocr.Reader(['ch_sim', 'en'], verbose=False)
        result = reader.readtext(path)
        print("############", result)
        loc_x_min = loc_x_max = loc_y_min = loc_y_max = 0
        for i in result:
            #The text in the recognition figure is prone to case error.
            #Compare the two texts in lower case to avoid the problem
            if i[1].lower() == text.lower():
                loc_x_min = i[0][0][0]
                loc_x_max = i[0][1][0]
                loc_y_min = i[0][0][1]
                loc_y_max = i[0][2][1]
                break
        loc_x = (loc_x_min + loc_x_max) / 2
        loc_y = (loc_y_min + loc_y_max) / 2
        return (loc_x,loc_y)

    def get_obtain_point_position(self, path, text):
        #path:Storage address of screenshot   text:The text content to look up
        self.driver.get_screenshot_as_file(path)
        reader = easyocr.Reader(['ch_sim', 'en'], verbose=False)
        result = reader.readtext(path)
        print("************************", result)
        loc_x_min = loc_x_max = loc_y_min = loc_y_max = 0
        for i in result:
            #The text in the recognition figure is prone to case error.
            #Compare the two texts in lower case to avoid the problem
            if text in i[1].lower():
                loc_x_min = i[0][0][0]
                loc_x_max = i[0][1][0]
                loc_y_min = i[0][0][1]
                loc_y_max = i[0][2][1]
                break
        loc_x = (loc_x_min + loc_x_max) / 2
        loc_y = (loc_y_min + loc_y_max) / 2
        return (loc_x, loc_y)

    def add_node(self,text):
        #text:Name of the child node to be added
        el = self.findElement(('id', 'visual_area'))
        loc = self.get_canvas_point_position('./Program_pictures/quanping.png', text)
        self.click_canvas_point(el, loc[0], loc[1])
        time.sleep(1)
        self.click(('id', 'add_child_node'))
        loc1 = self.get_canvas_point_position('./Program_pictures/addnode.png', 'node')
        self.click_canvas_point(el, loc1[0], loc1[1])
        return loc1

    def add_attr(self,text):
        #text:Name of the child attr to be added
        el = self.findElement(('id', 'visual_area'))
        loc = self.get_canvas_point_position('./Program_pictures/quanping.png', text)
        self.click_canvas_point(el, loc[0], loc[1])
        self.click(('id', 'add_child_attr'))
        self.driver.get_screenshot_as_file('./Program_pictures/addattr.png')
        reader = easyocr.Reader(['ch_sim', 'en'], verbose=False)
        result = reader.readtext('./Program_pictures/addattr.png')
        loc_x_min = loc_x_max = loc_y_min = loc_y_max = 0
        for i in result:
            #Query the location of the node whose initial letter is A (there is no node whose initial letter
            #is A on the current page, but the name of the node whose new attribute is attr_1).
            if 'a' == i[1].lower()[0]:
                loc_x_min = i[0][0][0]
                loc_x_max = i[0][1][0]
                loc_y_min = i[0][0][1]
                loc_y_max = i[0][2][1]
                break
        loc_x = (loc_x_min + loc_x_max) / 2
        loc_y = (loc_y_min + loc_y_max) / 2
        self.click_canvas_point(el, loc_x, loc_y)
        return (loc_x,loc_y)

    def node_in_kuang(self,path,text):
        #path:Storage address of screenshot   text:The text content to look up
        self.driver.get_screenshot_as_file(path)
        reader = easyocr.Reader(['ch_sim', 'en'], verbose=False)
        result = reader.readtext(path)
        flag = False
        for i in result:
            if i[1].lower() == text:
                flag = True
                break
        return flag

    def kuang_contain_text(self,path,text):
        #path:Storage address of screenshot   text:The text content to look up
        self.driver.get_screenshot_as_file(path)
        reader = easyocr.Reader(['ch_sim', 'en'], verbose=False)
        result = reader.readtext(path)
        flag = False
        for i in result:
            if text in i[1]:
                flag = True
                break
        return flag

    def text_count(self, path, text):
        # path:Storage address of screenshot   text:The text content to look up
        self.driver.get_screenshot_as_file(path)
        size = self.driver.get_window_size()
        size.get('width')
        Image.open(path).crop((0.1 * size.get('width'), 0, 0.6 * size.get('width'), 0.4 * size.get('height'))).save(
            path)
        reader = easyocr.Reader(['ch_sim', 'en'], verbose=False)
        result = reader.readtext(path)
        count = 0
        for i in result:
            if text in i[1]:
                count = count + 1
        return count