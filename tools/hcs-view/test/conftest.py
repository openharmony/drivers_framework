import pytest
from selenium import webdriver
from selenium.webdriver.chrome.webdriver import WebDriver

global_param = {}

@pytest.fixture
def _set_global_data(global_driver):
    driver = None
    global_param[global_driver] = driver

@pytest.fixture
def _get_global_data(global_driver):
    #print(f"通过_get_global_data获取global_driver值:{global_param.get(global_driver)}")
    return global_param.get(global_driver)

def pytest_addoption(parser):
    parser.addoption(
        "--browser", action="store", default="chrome", help="browser option: firefox or chrome"
             )

@pytest.fixture(scope='session')
def browser(request):
    driver = _get_global_data('global_driver')
    if driver is None:
        name = request.config.getoption("--browser")
        if name == "firefox":
            driver = webdriver.Firefox()
        elif name == "chrome":
            driver = webdriver.Chrome()
        else:
            driver = webdriver.Firefox()

    def fn():
        driver.quit()

    request.addfinalizer(fn)
    return driver
