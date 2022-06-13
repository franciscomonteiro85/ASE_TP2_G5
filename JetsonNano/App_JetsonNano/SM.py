class StateMachine:
    def __init__(self):
        self.label_on_entry: dict[str, list] = {}
        self.label_on_leave: dict[str, list] = {}
        self.transitions: dict[str, dict[str, list]] = {}
        self.current_label = ""

    def add_on_entry(self, label:str, callback):
        label = str(label)
        if label not in self.label_on_entry.keys():
            self.label_on_entry[label] = []
        self.label_on_entry[label].append(callback)

    def add_on_leave(self, label:str, callback):
        label = str(label)
        if label not in self.label_on_leave.keys():
            self.label_on_leave[label] = []
        self.label_on_leave[label].append(callback)

    def add_transition(self, label1:str, label2:str, callback):
        label1 = str(label1)
        label2 = str(label2)

        if label1 not in self.transitions:
            self.transitions[label1] = {label2: []}
        elif label2 not in self.transitions[label1]:
            self.transitions[label1][label2] = []

        self.transitions[label1][label2].append(callback)

    def process_event(self, label):
        if label == self.current_label:
            return

        if self.current_label != "" and self.current_label in self.label_on_leave:
            for f in self.label_on_leave[self.current_label]:
                f()

        if self.current_label in self.transitions and label in self.transitions[self.current_label]:
            for f in self.transitions[self.current_label][label]:
                f()

        if label in self.label_on_entry:
            for f in self.label_on_entry[label]:
                f()

        self.current_label = label

    def close(self):
        if self.current_label != "" and self.current_label in self.label_on_leave:
            for f in self.label_on_leave[self.current_label]:
                f()    