from PyQt5.QtWidgets import QMenu, QAction

class MenuService:
    def __init__(self, parent, menubar):
        self.parent = parent
        self.menubar = menubar

    def create_menus(self):
        # NGPS Menu Bar
        ngps_menu = self.menubar.addMenu('NGPS')

        # File Menu
        file_menu = self.menubar.addMenu('File')
        new_action = QAction('New', self.menubar)
        open_action = QAction('Open', self.menubar)
        save_action = QAction('Save', self.menubar)
        file_menu.addAction(new_action)
        file_menu.addAction(open_action)
        file_menu.addAction(save_action)

        # Edit Menu
        edit_menu = self.menubar.addMenu('Edit')
        undo_action = QAction('Undo', self.menubar)
        redo_action = QAction('Redo', self.menubar)
        cut_action = QAction('Cut', self.menubar)
        copy_action = QAction('Copy', self.menubar)
        paste_action = QAction('Paste', self.menubar)
        edit_menu.addAction(undo_action)
        edit_menu.addAction(redo_action)
        edit_menu.addAction(cut_action)
        edit_menu.addAction(copy_action)
        edit_menu.addAction(paste_action)

        # View Menu
        view_menu = self.menubar.addMenu('View')
        toggle_toolbar_action = QAction('Toggle Toolbar', self.menubar)
        view_menu.addAction(toggle_toolbar_action)

        # Tools Menu
        tools_menu = self.menubar.addMenu('Tools')
        settings_action = QAction('Settings', self.menubar)
        tools_menu.addAction(settings_action)
        
        # User Menu
        user_menu = self.menubar.addMenu('User')

        # Add Login Action
        login_action = QAction('Login', self.menubar)
        login_action.triggered.connect(self.parent.on_login)  # Connect to login handler
        user_menu.addAction(login_action)

        # Add Create Account Action
        create_account_action = QAction('Create Account', self.menubar)
        create_account_action.triggered.connect(self.parent.on_create_account)  # Connect to create account handler
        user_menu.addAction(create_account_action)

        # Help Menu
        help_menu = self.menubar.addMenu('Help')
        help_action = QAction('Help Contents', self.menubar)
        about_action = QAction('About', self.menubar)
        help_menu.addAction(help_action)
        help_menu.addAction(about_action)
