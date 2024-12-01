class TargetModel:
    def __init__(self):
        self.targets = []

    def add_target(self, target_data):
        self.targets.append(target_data)

    def get_targets(self):
        return self.targets

